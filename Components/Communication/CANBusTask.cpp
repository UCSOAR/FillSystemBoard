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


/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
extern FDCAN_HandleTypeDef hfdcan1;
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

    //init canbus these will have to be configured in the ioc
    moth = new CanAutoNodeMotherboard{&hfdcan1};
    //HAL_GPIO_WritePin(CAN_Standby_GPIO_Port, CAN_Standby_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void CANBusTask::Run(void * pvParams)
{
    while (1) {
    	RecieveData();

        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->Receive(cm);
        if(res) {
            HandleCommand(cm);
        }
        HAL_Delay(10);
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


void CANBusTask::RecieveData(){
	moth->CheckCANCommands();
	CanAutoNode::UniqueBoardID e = moth->GetIDOfBoardWithName("Thermocouple");

	if (moth->GetTicksSinceLastHeartbeat() > 1000)
	{
		moth->Heartbeat();
	}

	if (e != CanAutoNode::UniqueBoardID{0,0,0}) {

		uint8_t data[moth->GetSizeOfLogIndexInBoard(e, 0)];

		if(moth->ReadMessageFromDaughterByLogIndex(e, 0, data, sizeof(data))) {

		}
	}

}
