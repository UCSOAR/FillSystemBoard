/**
 ******************************************************************************
 * File Name          : DebugTask.cpp
 * Description        : Task for controlling debug input
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <SOARDebug/DebugTask.hpp>
#include "Command.hpp"
#include "CubeUtils.hpp"
#include <cstring>
#include "LoggingService.hpp"

#include "stm32h7xx_hal.h"
#include "FlashTask.hpp"
#include "FSBProtocolTask.hpp"
#include "KSZ9893R.hpp"
#include "main.h"

// External Tasks (to send debug commands to)

/* Macros --------------------------------------------------------------------*/

/* Structs -------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/
constexpr uint8_t DEBUG_TASK_PERIOD = 100;
extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi2;
extern ETH_HandleTypeDef heth;

/* Variables -----------------------------------------------------------------*/

/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief Constructor, sets all member variables
 */
DebugTask::DebugTask()
    : Task(TASK_DEBUG_QUEUE_DEPTH_OBJS), kUart_(DEFAULT_DEBUG_UART_DRIVER)
{
  memset(debugBuffer, 0, sizeof(debugBuffer));
  debugMsgIdx = 0;
  isDebugMsgReady = false;
}

/**
 * @brief Init task for RTOS
 */
void DebugTask::InitTask()
{
  // Make sure the task is not already initialized
  SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Debug task twice");

  // Start the task
  BaseType_t rtValue = xTaskCreate(
      (TaskFunction_t)DebugTask::RunTask, (const char *)"DebugTask",
      (uint16_t)TASK_DEBUG_STACK_DEPTH_WORDS, (void *)this,
      (UBaseType_t)TASK_DEBUG_PRIORITY, (TaskHandle_t *)&rtTaskHandle);

  // Ensure creation succeded
  SOAR_ASSERT(rtValue == pdPASS, "DebugTask::InitTask - xTaskCreate() failed");
}

// TODO: Only run thread when appropriate GPIO pin pulled HIGH (or by define)
/**
 *    @brief Runcode for the DebugTask
 */
void DebugTask::Run(void *pvParams)
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
        cm.GetTaskCommand() == EVENT_DEBUG_RX_COMPLETE)
    {
      HandleDebugMessage((const char *)debugBuffer);
    }

    cm.Reset();
  }
}

/**
 * @brief Handles debug messages, assumes msg is null terminated
 * @param msg Message to read, must be null termianted
 */
void DebugTask::HandleDebugMessage(const char *msg)
{
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
    SOAR_PRINT("Debug Task Runtime  \t: %d ms\n\n",
               TICKS_TO_MS(xTaskGetTickCount()));
  } 











  else if (strcmp(msg, "switch") == 0) {
    uint8_t ksz_id[4] = {0};
    HAL_StatusTypeDef ksz_st = ksz9893::readReg(&hspi2, CS_SWITCH_GPIO_Port, CS_SWITCH_Pin,
                                                0x0000, ksz_id, sizeof(ksz_id));
    SOAR_PRINT("KSZ9893R read status: %d, ID bytes: %02X %02X %02X %02X\n",
               (int)ksz_st, ksz_id[0], ksz_id[1], ksz_id[2], ksz_id[3]);

    for (uint8_t addr = 1; addr <= 3; ++addr) {
        uint16_t id1 = 0, id2 = 0, status = 0;

        
        HAL_GPIO_WritePin(CS_SWITCH_GPIO_Port, CS_SWITCH_Pin, GPIO_PIN_RESET);
        HAL_Delay(300);
        HAL_StatusTypeDef st1 = ksz9893::readPhyRegSpi(&hspi2, CS_SWITCH_GPIO_Port, CS_SWITCH_Pin, addr, 2, &id1);
        HAL_StatusTypeDef st2 = ksz9893::readPhyRegSpi(&hspi2, CS_SWITCH_GPIO_Port, CS_SWITCH_Pin, addr, 3, &id2);
        HAL_StatusTypeDef st3 = ksz9893::readPhyRegSpi(&hspi2, CS_SWITCH_GPIO_Port, CS_SWITCH_Pin, addr, 1, &status);
        HAL_Delay(3000);
        HAL_GPIO_WritePin(CS_SWITCH_GPIO_Port, CS_SWITCH_Pin, GPIO_PIN_SET);

        SOAR_PRINT(
            "PHY addr=%02u | "
            "ID1=%04X (HAL=%d) | "
            "ID2=%04X (HAL=%d) | "
            "STATUS=%04X (HAL=%d)\n",
            addr, id1, st1, id2, st2, status, st3
        );

        if (st1 != HAL_OK || st2 != HAL_OK || st3 != HAL_OK) {
            SOAR_PRINT("  -> HAL read failure\n");
            continue;
        }

        if (id1 == 0xFFFF && id2 == 0xFFFF) {
            SOAR_PRINT("  -> Invalid IDs: no valid PHY response detected\n");
            continue;
        }

        if (id1 == 0x0000 && id2 == 0x0000) {
            SOAR_PRINT("  -> Invalid IDs: both identifiers are zero\n");
            continue;
        }

        SOAR_PRINT("  -> Nontrivial PHY ID detected\n");

        if (status == 0xFFFF) {
            SOAR_PRINT("  -> Status invalid: all bits are set\n");
        } else {
            SOAR_PRINT("  -> Link: %s\n",
                       (status & 0x0004) ? "UP" : "DOWN");
        }
    }
}


//   else if (strcmp(msg, "switch") == 0) {
//     // uint8_t ksz_id[4] = {0};
//     // HAL_StatusTypeDef ksz_st = ksz9893::readReg(&hspi2, CS_SWITCH_GPIO_Port, CS_SWITCH_Pin,
//     //                                             0x0000, ksz_id, sizeof(ksz_id));
//     // SOAR_PRINT("KSZ9893R read status: %d, ID bytes: %02X %02X %02X %02X\n",
//     //            (int)ksz_st, ksz_id[0], ksz_id[1], ksz_id[2], ksz_id[3]);


//     for (uint8_t addr = 0; addr < 32; ++addr) {
//     uint16_t id1 = 0, id2 = 0, status = 0;

//     HAL_StatusTypeDef st1 = ksz9893::readPhyReg(&heth, addr, 2, &id1);
//     HAL_StatusTypeDef st2 = ksz9893::readPhyReg(&heth, addr, 3, &id2);
//     HAL_StatusTypeDef st3 = ksz9893::readPhyReg(&heth, addr, 1, &status);

//     SOAR_PRINT(
//         "PHY addr=%02u | "
//         "ID1=%04X (HAL=%d) | "
//         "ID2=%04X (HAL=%d) | "
//         "STATUS=%04X (HAL=%d)\n",
//         addr, id1, st1, id2, st2, status, st3
//     );

//     if (st1 != HAL_OK || st2 != HAL_OK || st3 != HAL_OK) {
//         SOAR_PRINT("  -> HAL read failure\n");
//         continue;
//     }

//     if (id1 == 0xFFFF && id2 == 0xFFFF) {
//         SOAR_PRINT("  -> Invalid IDs: no valid PHY response detected\n");
//         continue;
//     }

//     if (id1 == 0x0000 && id2 == 0x0000) {
//         SOAR_PRINT("  -> Invalid IDs: both identifiers are zero\n");
//         continue;
//     }

//     SOAR_PRINT("  -> Nontrivial PHY ID detected\n");

//     if (status == 0xFFFF) {
//         SOAR_PRINT("  -> Status invalid: all bits are set\n");
//     } else {
//         SOAR_PRINT("  -> Link: %s\n",
//                    (status & 0x0004) ? "UP" : "DOWN");
//     }
// }


//   }






  else if (strcmp(msg, "flash_test") == 0)
  {
    SOAR_PRINT("Debug: Triggering flash tests\n");
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
  else if (strncmp(msg, "transmit ", strlen("transmit ")) == 0)
  {
      const char* radioMsg = &msg[strlen("transmit ")];
      UART::RADIO->Transmit((uint8_t*)radioMsg, strlen(radioMsg));
      UART::RADIO->Transmit((uint8_t*)"\r\n", 2);
  }
  else if (strcmp(msg, "transmit") == 0)
  {
      UART::RADIO->Transmit((uint8_t*)"Yo\r\n", 4);
  }
  else if (strncmp(msg, "proto_fcb ", strlen("proto_fcb ")) == 0)
  {
      const int32_t command = ExtractIntParameter(msg, strlen("proto_fcb "));
      if (command != ERRVAL)
      {
          FSBProtocolTask::Inst().SendFcbRadioCommand(
              static_cast<Proto::FcbCommand::Command>(command));
      }
  }
  else if (strcmp(msg, "fcb_abort") == 0)
  {
      FSBProtocolTask::Inst().SendFcbRadioCommand(
          Proto::FcbCommand::Command::RSC_ANY_TO_ABORT);
  }
  else if (strcmp(msg, "fcb_open_vent") == 0)
  {
      FSBProtocolTask::Inst().SendFcbRadioCommand(
          Proto::FcbCommand::Command::RSC_OPEN_VENT);
  }
  else if (strcmp(msg, "fcb_close_vent") == 0)
  {
      FSBProtocolTask::Inst().SendFcbRadioCommand(
          Proto::FcbCommand::Command::RSC_CLOSE_VENT);
  }


  else
  {
    // Single character command, or unknown command
    switch (msg[0])
    {
    default:
      SOAR_PRINT("Debug, unknown command: %s\n", msg);
      break;
    }
  }

  // We've read the data, clear the buffer
  debugMsgIdx = 0;
  isDebugMsgReady = false;
}

/**
 * @brief Receive data, currently receives by arming interrupt
 */
bool DebugTask::ReceiveData() { return kUart_->ReceiveIT(&debugRxChar, this); }

/**
 * @brief Receive data to the buffer
 * @return Whether the debugBuffer is ready or not
 */
void DebugTask::InterruptRxData(uint8_t errors)
{
  // If we already have an unprocessed debug message, ignore this byte
  if (!isDebugMsgReady)
  {
    // Check byte for end of message - note if using termite you must turn on
    // append CR
    if (debugRxChar == '\r' || debugRxChar == '\n' ||
        debugMsgIdx == DEBUG_RX_BUFFER_SZ_BYTES)
    {
      if (debugMsgIdx > 0)
      {
        // Null terminate and process
        debugBuffer[debugMsgIdx++] = '\0';
        isDebugMsgReady = true;

        // Notify the debug task
        Command cm(DATA_COMMAND, EVENT_DEBUG_RX_COMPLETE);
        bool res = qEvtQueue->SendFromISR(cm);

        // If we failed to send the event, we should reset the buffer, that way
        // DebugTask doesn't stall
        if (res == false)
        {
          debugMsgIdx = 0;
          isDebugMsgReady = false;
        }
      }
    }
    else
    {
      debugBuffer[debugMsgIdx++] = debugRxChar;
    }
  }

  // Re-arm the interrupt
  ReceiveData();
}

/* Helper Functions
 * --------------------------------------------------------------*/
/**
 * @brief Extracts an integer parameter from a string
 * @brief msg Message to extract from, MUST be at least identifierLen long, and
 * properly null terminated
 * @brief identifierLen Length of the identifier eg. 'rsc ' (Including the
 * space) is 4
 * @return ERRVAL on failure, otherwise the extracted value
 */
int32_t DebugTask::ExtractIntParameter(const char *msg,
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
