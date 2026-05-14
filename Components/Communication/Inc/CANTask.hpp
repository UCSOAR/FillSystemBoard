/**
 ******************************************************************************
 * File Name          : CANTask.hpp
 * Description        : canbus manager
 ******************************************************************************
 */
#ifndef CUBE_SYSTEM_CAN_TASK_HPP_
#define CUBE_SYSTEM_CAN_TASK_HPP_

/* Includes ------------------------------------------------------------------*/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include <stdint.h>
#include "CanAutoNodeMotherboard.hpp"
#include "RPBLogs.hpp"
#include "CANBusTargets.hpp"
#include "SOLLogs.hpp"


/* Enums ------------------------------------------------------------------*/
enum RPB_TASK_COMMANDS
{
    RPB_TASK_COMMAND_NONE = 0,
    EVENT_RPB_INIT,
    EVENT_RPB_TEST,
    EVENT_RPB_LOG_DATA,
    EVENT_RPB_CLEANUP
};

/* Macros ------------------------------------------------------------------*/

extern FDCAN_HandleTypeDef hfdcan1;
/* Class ------------------------------------------------------------------*/
class CANTask : public Task
{
public:
    static CANTask &Inst()
    {
        static CANTask inst;
        return inst;
    }

    void InitTask();


protected:
    static void RunTask(void *pvParams)
    {
        CANTask::Inst().Run(pvParams);
    } // Static Task Interface, passes control to the instance Run();

    void Run(void *pvParams); // Main run code
    void HandleCommand(Command &cm);
    bool HandleCANCommands();

private:
    // Private Functions
    CANTask();                                  // Private constructor
    CANTask(const CANTask &);            // Prevent copy-construction
    CANTask &operator=(const CANTask &); // Prevent assignment

    CanAutoNodeMotherboard motherboard = {&hfdcan1};

};

#endif // CUBE_SYSTEM_RPB_TASK_HPP_
