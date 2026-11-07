/**
 ********************************************************************************
 * @file    FlashMapTestTask.hpp
 * @author  Jacob Plourde (Jacobplo)
 * @date    November 7, 2027
 * @brief   This task is used to the FlashMap module.
 ********************************************************************************
 */

#ifndef FLASH_MAP_TEST_TASK_HPP
#define FLASH_MAP_TEST_TASK_HPP

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class FlashMapTestTask : public Task
{
public:
    static FlashMapTestTask& Inst() {
        static FlashMapTestTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { FlashMapTestTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);

private:
    // Private Functions
    FlashMapTestTask();        // Private constructor
    FlashMapTestTask(const FlashMapTestTask&);                        // Prevent copy-construction
    FlashMapTestTask& operator=(const FlashMapTestTask&);            // Prevent assignment
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FLASH_MAP_TEST_TASK_HPP */
