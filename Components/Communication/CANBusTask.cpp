/*
 * CANBusTask.cpp
 *
 *  Created on: Apr 28, 2026
 *      Author: jaddina
 */
/**
 ********************************************************************************
 * @file    ${file_name}
 * @author  ${user}
 * @date    ${date}
 * @brief   This is a template source file to create a new task in our firmware
 *
 * Setup Steps
 * 1. Define the Task Queue Depth in SystemDefines.hpp
 * 2. Define the Task Stack Depth in SystemDefines.hpp
 * 3. Define the Task Priority in SystemDefines.hpp
 * 4. Replace all placeholders marked with a $ sign
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "CANBusTask.hpp"
#include "SystemDefines.hpp"
#include "CANBusTargets.hpp"


/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
extern FDCAN_HandleTypeDef hfdcan1;

namespace {
constexpr uint8_t kFillSystemBoardType = 1;
constexpr uint8_t kFillSystemBoardSlot = 1;
constexpr uint32_t kCanServicePeriodMs = 10;

enum CANBusLogIndexes : uint8_t {
    CAN_LOG_IMU_9_AXIS = 0,
    CAN_LOG_IMU_6_AXIS,
    CAN_LOG_ACCEL,
    CAN_LOG_GYRO,
    CAN_LOG_MAG,
    CAN_LOG_COUNT
};

const CanAutoNodeDaughter::LogInit kDaughterLogs[] = {
    {sizeof(CanImuData9Axis)},
    {sizeof(CanImuData6Axis)},
    {sizeof(CanAccelData)},
    {sizeof(CanGyroData)},
    {sizeof(CanMagData)},
};
}
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
 * @brief Constructor for CANBusTask
 */
CANBusTask::CANBusTask() : Task(TASK_LOGGING_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the CANBusTask
 *        Do not modify this function aside from adding the task name
 */
void CANBusTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)CANBusTask::RunTask,
            (const char*)"CANBusTask",
            (uint16_t){TASK_LOGGING_QUEUE_DEPTH_WORDS},
            (void*)this,
            (UBaseType_t){TASK_LOGGING_PRIORITY},
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "CANBusTask::InitTask() - xTaskCreate() failed");

    daughter = new CanAutoNodeDaughter(&hfdcan1,
        kDaughterLogs,
        sizeof(kDaughterLogs) / sizeof(kDaughterLogs[0]),
        kFillSystemBoardType,
        kFillSystemBoardSlot,
        CAN_GSE_TARGET_FSB);
    SOAR_ASSERT(daughter != nullptr, "CANBusTask::InitTask() - CAN daughter alloc failed");
    //HAL_GPIO_WritePin(CAN_Standby_GPIO_Port, CAN_Standby_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void CANBusTask::Run(void * pvParams)
{
    while (1) {
    	ServiceCanNetwork();

        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->Receive(cm, kCanServicePeriodMs);
        if(res) {
            HandleCommand(cm);
        }
    }
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void CANBusTask::HandleCommand(Command& cm)
{
    switch (cm.GetCommand()) {

    default:
        SOAR_PRINT("CANBusTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}


void CANBusTask::ServiceCanNetwork()
{
    if (daughter == nullptr) {
        return;
    }

    if (daughter->GetCurrentState() == CanAutoNodeDaughter::ERROR) {
        SOAR_PRINT("CANBusTask - CAN daughter node entered ERROR state\n");
        return;
    }

    (void)daughter->CheckCANCommands();

    while (daughter->GetCurrentState() == CanAutoNodeDaughter::UNINITIALIZED) {
        SOAR_PRINT("CANBusTask - requesting to join CAN network\n");
        (void)daughter->TryRequestingJoiningNetwork();
    }

    if (daughter->GetCurrentState() == CanAutoNodeDaughter::READY) {
        ReceiveData();
    }
}

void CANBusTask::ReceiveData()
{
    (void)ReceiveByIndex<CanImuData9Axis>(CAN_LOG_IMU_9_AXIS);
    (void)ReceiveByIndex<CanImuData6Axis>(CAN_LOG_IMU_6_AXIS);
    (void)ReceiveByIndex<CanAccelData>(CAN_LOG_ACCEL);
    (void)ReceiveByIndex<CanGyroData>(CAN_LOG_GYRO);
    (void)ReceiveByIndex<CanMagData>(CAN_LOG_MAG);
}

template <typename T>
bool CANBusTask::ReceiveByIndex(uint8_t logIndex)
{
    T data{};
    if (!daughter->ReadMessageByLogIndex(logIndex, reinterpret_cast<uint8_t*>(&data), sizeof(data))) {
        return false;
    }

    HandleReceivedCanMessage(logIndex, data);
    return true;
}

void CANBusTask::HandleReceivedCanMessage(uint8_t logIndex, const CanImuData9Axis& data)
{
    SOAR_PRINT("CANBusTask - RX log %u IMU9 accel=(%lu,%lu,%lu) gyro=(%lu,%lu,%lu) mag=(%lu,%lu,%lu)\n",
        static_cast<unsigned int>(logIndex),
        static_cast<unsigned long>(data.accel_x),
        static_cast<unsigned long>(data.accel_y),
        static_cast<unsigned long>(data.accel_z),
        static_cast<unsigned long>(data.gyro_x),
        static_cast<unsigned long>(data.gyro_y),
        static_cast<unsigned long>(data.gyro_z),
        static_cast<unsigned long>(data.mag_x),
        static_cast<unsigned long>(data.mag_y),
        static_cast<unsigned long>(data.mag_z));
}

void CANBusTask::HandleReceivedCanMessage(uint8_t logIndex, const CanImuData6Axis& data)
{
    SOAR_PRINT("CANBusTask - RX log %u IMU6 accel=(%lu,%lu,%lu) gyro=(%lu,%lu,%lu)\n",
        static_cast<unsigned int>(logIndex),
        static_cast<unsigned long>(data.accel_x),
        static_cast<unsigned long>(data.accel_y),
        static_cast<unsigned long>(data.accel_z),
        static_cast<unsigned long>(data.gyro_x),
        static_cast<unsigned long>(data.gyro_y),
        static_cast<unsigned long>(data.gyro_z));
}

void CANBusTask::HandleReceivedCanMessage(uint8_t logIndex, const CanAccelData& data)
{
    SOAR_PRINT("CANBusTask - RX log %u accel=(%lu,%lu,%lu)\n",
        static_cast<unsigned int>(logIndex),
        static_cast<unsigned long>(data.accel_x),
        static_cast<unsigned long>(data.accel_y),
        static_cast<unsigned long>(data.accel_z));
}

void CANBusTask::HandleReceivedCanMessage(uint8_t logIndex, const CanGyroData& data)
{
    SOAR_PRINT("CANBusTask - RX log %u gyro=(%lu,%lu,%lu)\n",
        static_cast<unsigned int>(logIndex),
        static_cast<unsigned long>(data.gyro_x),
        static_cast<unsigned long>(data.gyro_y),
        static_cast<unsigned long>(data.gyro_z));
}

void CANBusTask::HandleReceivedCanMessage(uint8_t logIndex, const CanMagData& data)
{
    SOAR_PRINT("CANBusTask - RX log %u mag=(%lu,%lu,%lu)\n",
        static_cast<unsigned int>(logIndex),
        static_cast<unsigned long>(data.mag_x),
        static_cast<unsigned long>(data.mag_y),
        static_cast<unsigned long>(data.mag_z));
}
