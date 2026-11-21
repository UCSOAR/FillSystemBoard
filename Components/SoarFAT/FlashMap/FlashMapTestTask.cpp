/**
 ********************************************************************************
 * @file    FlashMapTestTask.cpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    November 7, 2027
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/

#include "FlashMapTestTask.hpp"
#include "FlashMap.hpp"

#include "Flash.hpp"
#include "FlashMX66XX.hpp"
#include "SystemDefines.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

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
 * @brief Constructor for FlashMapTestTask
 */
FlashMapTestTask::FlashMapTestTask() : Task(TASK_FLASH_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the FlashMapTestTask
 *        Do not modify this function aside from adding the task name
 */
void FlashMapTestTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)FlashMapTestTask::RunTask,
            (const char*)"FlashMapTestTask",
            (uint16_t)TASK_FLASH_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_FLASH_TASK_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "FlashMapTestTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void FlashMapTestTask::Run(void * pvParams)
{
    static FlashMX66XX& flash = FlashMX66XX::Inst();
    FlashMap<FlashMX66XX> map(flash);

    map.Init();

    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res) {
            HandleCommand(cm);
        }
    }
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void FlashMapTestTask::HandleCommand(Command& cm)
{
    switch (cm.GetCommand()) {

    default:
        SOAR_PRINT("FlashMapTestTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}
