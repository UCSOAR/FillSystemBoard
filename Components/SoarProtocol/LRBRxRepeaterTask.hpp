/**
 ******************************************************************************
 * File Name          : LRBRxRepeaterTask.hpp
 * Description        : LRBRx UART line repeater task
 *
 *                      This is intended to be used as a template for creating
 *                      specific UART line repeater tasks. Guideline:
 *   1. Copy this file while replacing all 'Example' with '{your uart name}', either manually or with this:
 *      sed 's/Example/{your uart name}/g' ExampleRepeaterTask.hpp > {your uart name}RepeaterTask.hpp
 *   2. Change the Configuration section below
 *      edit    Example_REPEATER_TASK_HUART with your UART handle
 *      and the Example_UART_TASK_COMMAND with your UART task command
 *   3. Add:
 *      else if (huart->Instance == {edited uart handle}->Instance)
 *           ExampleRepeaterTask::Inst().InterruptRxData();
 *      to HAL_UART_RxCpltCallback in the file that contains the callback
 *   4. Add:
 *      #include "ExampleRepeaterTask.hpp"
 *      and
 *      ExampleRepeaterTask::Inst().InitTask()
 *      to main_avionics.cpp
 *   5. Delete this extended description (if you want)
 *   6. Delete this file (once done with steps 1-5 for all UARTs you need to repeat)
 ******************************************************************************
 */
#include "RepeaterTask.hpp"
#include "UARTTask.hpp"

/* Configuration ------------------------------------------------------------------*/
constexpr uint16_t LRBRx_REPEATER_TASK_UART_TASK_COMMAND = UART_TASK_COMMAND_SEND_PI;

/* Class ------------------------------------------------------------------*/
class LRBRxRepeaterTask : public RepeaterTask
{
public:
    static LRBRxRepeaterTask &Inst()
    {
        static LRBRxRepeaterTask inst;
        return inst;
    }

    void InitTask()
    {
        // Make sure the task is not already initialized
        SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot init LRBRxRptr task twice");

        // Start the task
        BaseType_t rtValue =
            xTaskCreate((TaskFunction_t)LRBRxRepeaterTask::RunTask,
                        (const char *)"LRBRxRptr",
                        (uint16_t)TASK_REPEATER_STACK_DEPTH_WORDS,
                        (void *)this,
                        (UBaseType_t)TASK_REPEATER_PRIORITY,
                        (TaskHandle_t *)&rtTaskHandle);

        // Ensure creation succeded
        SOAR_ASSERT(rtValue == pdPASS, "LRBRxRptr xTaskCreate() failed");
    }

protected:
    static void RunTask(void *pvParams) { LRBRxRepeaterTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

private:
    LRBRxRepeaterTask() : RepeaterTask(UART::LRB, LRBRx_REPEATER_TASK_UART_TASK_COMMAND) {}
    LRBRxRepeaterTask(const LRBRxRepeaterTask &);            // Prevent copy-construction
    LRBRxRepeaterTask &operator=(const LRBRxRepeaterTask &); // Prevent assignment
};
