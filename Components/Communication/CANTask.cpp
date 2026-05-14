/**
 ******************************************************************************
 * File Name          : CANTask.cpp
 * Description        : File System Task implementation for USB storage operations
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <Communication/Inc/CANTask.hpp>
#include "SystemDefines.hpp"
#include <stdint.h>
#include "stm32h7xx_hal.h"


/**
 * @brief Constructor, sets up task
 */
CANTask::CANTask() : Task(TASK_CAN_QUEUE_DEPTH_OBJS)
{

}

/**
 * @brief Initialize the CANTask
 */
void CANTask::InitTask()
{
	// Make sure the task is not already initialized
	SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize CAN task twice");

	// Start the task
	BaseType_t rtValue =
			xTaskCreate((TaskFunction_t)CANTask::RunTask,
					(const char *)"CANTask",
					(uint16_t)TASK_CAN_STACK_DEPTH_WORDS,
					(void *)this,
					(UBaseType_t)TASK_CAN_PRIORITY,
					(TaskHandle_t *)&rtTaskHandle);

	// Ensure creation succeded
	SOAR_ASSERT(rtValue == pdPASS, "CANTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the CAN Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void CANTask::Run(void *pvParams)
{
	SOAR_PRINT("CANTask::Run() - Starting task\n");

	osDelay(500);


	// Receive heartbeat from daughter boards
	if (motherboard.GetTicksSinceLastHeartbeat() > 500) {
		motherboard.Heartbeat();
	}
	int counter = 0;
	while (1)
	{
		/* Process commands in blocking mode */
		motherboard.CheckCANCommands();




			if (counter == 150){
				counter = 0;
				SOL_POWER_COMMAND command;
				command.SolOn = true;
				motherboard.SendMessageToNameByLogIndex(CAN_GSE_TARGET_SOL1, _SOL_POWER_COMMAND_LOGINDEX, (uint8_t*)&command);
				SOAR_PRINT("sending message to SOL");

			}
			counter++;

		Command cm;
		bool res = qEvtQueue->Receive(cm);
		if (res)
		{
			HandleCommand(cm);
		}

		osDelay(10);

		HandleCANCommands();



		}

	}



/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void CANTask::HandleCommand(Command &cm)
{
	// Handle task-specific commands
	if (cm.GetCommand() == TASK_SPECIFIC_COMMAND)
	{
		switch (cm.GetTaskCommand())
		{

		default:
			SOAR_PRINT("CANTask - Received Unsupported Task Command {%d}\n", cm.GetTaskCommand());
			break;
		}
	}
	else
	{
		// Handle global commands if any
		SOAR_PRINT("CANTask - Received Unsupported Global Command {%d}\n", cm.GetCommand());
	}

	// No matter what happens, we must reset allocated data
	cm.Reset();
}

