/**
 ********************************************************************************
 * @file    RadioTask.cpp
 * @author  spiro
 * @date    Apr 23, 2026
 * @brief   Task for controlling radio input
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "RadioTask.hpp"
#include "Command.hpp"
#include "CubeUtils.hpp"
#include <cstring>
#include "LoggingService.hpp"

#include "stm32h7xx_hal.h"
#include "FlashTask.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
constexpr uint8_t RADIO_TASK_PERIOD = 100;
extern I2C_HandleTypeDef hi2c2;

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
/**
 * @brief Constructor, sets all member variables
 */
RadioTask::RadioTask()
    : Task(TASK_RADIO_QUEUE_DEPTH_OBJS), kUart_(UART::RADIO)
{
  memset(radioBuffer, 0, sizeof(radioBuffer));
  radioMsgIdx = 0;
  isRadioMsgReady = false;
}

/**
 * @brief Init task for RTOS
 */
void RadioTask::InitTask()
{
  // Make sure the task is not already initialized
  SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Radio task twice");

  // Start the task
  BaseType_t rtValue = xTaskCreate(
      (TaskFunction_t)RadioTask::RunTask, (const char *)"RadioTask",
      (uint16_t)TASK_RADIO_STACK_DEPTH_WORDS, (void *)this,
      (UBaseType_t)TASK_RADIO_PRIORITY, (TaskHandle_t *)&rtTaskHandle);

  // Ensure creation succeeded
  SOAR_ASSERT(rtValue == pdPASS, "RadioTask::InitTask - xTaskCreate() failed");
}

/**
 * @brief Runcode for the RadioTask
 */
void RadioTask::Run(void *pvParams)
{
  // Arm the interrupt
  ReceiveData();

  while (1)
  {
    Command cm;

    // Wait forever for a command
    qEvtQueue->ReceiveWait(cm);

    // Process the command
    if (cm.GetCommand() == DATA_COMMAND &&
        cm.GetTaskCommand() == EVENT_RADIO_RX_COMPLETE)
    {
      HandleRadioMessage((const char *)radioBuffer);
    }

    cm.Reset();
  }
}

/**
 * @brief Handles radio messages, assumes msg is null terminated
 * @param msg Message to read, must be null terminated
 */
void RadioTask::HandleRadioMessage(const char *msg)
{
  SOAR_PRINT("%s\r\n", msg);

  //-- SYSTEM / CHAR COMMANDS -- (Must be last)
  if (strcmp(msg, "sysreset") == 0)
  {
    // Reset the system
    SOAR_ASSERT(false, "System reset requested");
  }
  else if (strcmp(msg, "sysinfo") == 0)
  {
    // Print message
    SOAR_PRINT("\n\n-- CUBE SYSTEM --\n");
    SOAR_PRINT("Current System Free Heap: %d Bytes\n", xPortGetFreeHeapSize());
    SOAR_PRINT("Lowest Ever Free Heap: %d Bytes\n",
               xPortGetMinimumEverFreeHeapSize());
    SOAR_PRINT("Radio Task Runtime \t: %d ms\n\n",
               TICKS_TO_MS(xTaskGetTickCount()));
  }
//  else if (strcmp(msg, "imu1") == 0)
//  {
//
//    SOAR_PRINT("Radio Imu 32G single sample");
//    Command cmd(DATA_COMMAND, IMUTask::IMU_SAMPLE_AND_LOG);
//    IMUTask::Inst().GetEventQueue()->Send(cmd);
//  }
//  else if (strcmp(msg, "imu1loop") == 0)
//  {
//
//    SOAR_PRINT("Radio Imu 32G continuous read start");
//    Command cmd(DATA_COMMAND, IMUTask::IMU_START_CONTINUOUS_PRINT);
//    IMUTask::Inst().GetEventQueue()->Send(cmd);
//  }
//  else if (strcmp(msg, "imu1stop") == 0)
//  {
//
//    SOAR_PRINT("Radio Imu 32G continuous read stop");
//    Command cmd(DATA_COMMAND, IMUTask::IMU_STOP_CONTINUOUS_PRINT);
//    IMUTask::Inst().GetEventQueue()->Send(cmd);
//  }
//  else if (strcmp(msg, "imu2") == 0)
//  {
//
//    SOAR_PRINT("Radio Imu 16G single sample");
//    Command cmd(DATA_COMMAND, LSM6DSOTask::IMU_SAMPLE_AND_LOG);
//    LSM6DSOTask::Inst().GetEventQueue()->Send(cmd);
//  }
//  else if (strcmp(msg, "imu2loop") == 0)
//  {
//
//    SOAR_PRINT("Radio Imu 16G continuous read start");
//    Command cmd(DATA_COMMAND, LSM6DSOTask::IMU_START_CONTINUOUS_PRINT);
//    LSM6DSOTask::Inst().GetEventQueue()->Send(cmd);
//  }
//  else if (strcmp(msg, "imu2stop") == 0)
//  {
//
//    SOAR_PRINT("Radio Imu 16G continuous read stop");
//    Command cmd(DATA_COMMAND, LSM6DSOTask::IMU_STOP_CONTINUOUS_PRINT);
//    LSM6DSOTask::Inst().GetEventQueue()->Send(cmd);
//  }

  else if (strcmp(msg, "baro1") == 0)
  {
    SOAR_PRINT("Radio Baro07 read");
    Command cmd(DATA_COMMAND, BARO07_SAMPLE_AND_LOG);
    BaroTask07::Inst().GetEventQueue()->Send(cmd);
  }
  else if (strcmp(msg, "baro2") == 0)
  {
    SOAR_PRINT("Radio Baro11 read");
    Command cmd(DATA_COMMAND, BARO11_SAMPLE_AND_LOG);
    BaroTask11::Inst().GetEventQueue()->Send(cmd);
  }
  else if (strcmp(msg, "mag") == 0)
  {
    SOAR_PRINT("Radio mag read");
    Command cmd(DATA_COMMAND, MMC5983MATask::MMC_CMD_ENABLE_LOG);
    MMC5983MATask::Inst().GetEventQueue()->Send(cmd);
  }
  else if (strcmp(msg, "flash_test") == 0)
  {
    SOAR_PRINT("Radio: Triggering flash tests\n");
    FlashTask::Inst().TriggerTest();
  }
  else if (strcmp(msg, "flash_dump") == 0)
  {
    Command cmd(TASK_SPECIFIC_COMMAND, FLASH_DUMP);
    FlashTask::Inst().GetEventQueue()->Send(cmd);
  }
  else if (strcmp(msg, "stop_dump") == 0)
  {
    LoggingService::StopDump();
  }
  else if (strcmp(msg, "transmit") == 0)
  {
    UART::RADIO->Transmit((unsigned char *)"Yo", 2);
  }
  else
  {
    // Single character command, or unknown command
    switch (msg[0])
    {
    default:
      SOAR_PRINT("Radio, unknown command: %s\n", msg);
      break;
    }
  }

  // We've read the data, clear the buffer
  radioMsgIdx = 0;
  isRadioMsgReady = false;
}

/**
 * @brief Receive data, currently receives by arming interrupt
 */
bool RadioTask::ReceiveData() { return kUart_->ReceiveIT(&radioRxChar, this); }

/**
 * @brief Receive data to the buffer
 * @return Whether the radioBuffer is ready or not
 */
void RadioTask::InterruptRxData(uint8_t errors)
{
  // If we already have an unprocessed radio message, ignore this byte
  if (!isRadioMsgReady)
  {
    // Check byte for end of message - note if using termite you must turn on
    // append CR
    if (radioRxChar == '\r' || radioRxChar == '\n' ||
        radioMsgIdx == RADIO_RX_BUFFER_SZ_BYTES)
    {
      if (radioMsgIdx > 0)
      {
        // Null terminate and process
        radioBuffer[radioMsgIdx++] = '\0';
        isRadioMsgReady = true;

        // Notify the radio task
        Command cm(DATA_COMMAND, EVENT_RADIO_RX_COMPLETE);
        bool res = qEvtQueue->SendFromISR(cm);

        // If we failed to send the event, we should reset the buffer, that way
        // RadioTask doesn't stall
        if (res == false)
        {
          radioMsgIdx = 0;
          isRadioMsgReady = false;
        }
      }
    }
    else
    {
      radioBuffer[radioMsgIdx++] = radioRxChar;
    }
  }

  // Re-arm the interrupt
  ReceiveData();
}

/**
 * @brief Extracts an integer parameter from a string
 * @brief msg Message to extract from, MUST be at least identifierLen long, and
 * properly null terminated
 * @brief identifierLen Length of the identifier eg. 'rsc ' (Including the
 * space) is 4
 * @return ERRVAL on failure, otherwise the extracted value
 */
int32_t RadioTask::ExtractIntParameter(const char *msg,
                                       uint16_t identifierLen)
{
  // Handle a command with an int parameter at the end
  if (static_cast<uint16_t>(strlen(msg)) < identifierLen + 1)
  {
    SOAR_PRINT("Int parameter command insufficient length\r\n");
    return ERRVAL;
  }

  // Extract the value and attempt conversion to integer
  const int32_t val = Utils::StringToLong(&msg[identifierLen]);
  if (val == ERRVAL)
  {
    SOAR_PRINT("Int parameter command invalid value\r\n");
  }

  return val;
}
