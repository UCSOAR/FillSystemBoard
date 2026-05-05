/*
 * CANBusTask.hpp
 *
 *  Created on: Apr 28, 2026
 *      Author: jaddina
 */

#ifndef CANBUSTASK_HPP_
#define CANBUSTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "CanAutoNodeDaughter.hpp"
#include "LogStructs.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class CANBusTask : public Task
{
public:
    static CANBusTask& Inst() {
        static CANBusTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { CANBusTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);

private:
    // Private Functions
    CANBusTask();        // Private constructor
    CANBusTask(const CANBusTask&);                        // Prevent copy-construction
    CANBusTask& operator=(const CANBusTask&); 			// Prevent assignment
    void ServiceCanNetwork();
    void ReceiveData();
    void HandleReceivedCanMessage(uint8_t logIndex, const CanImuData9Axis& data);
    void HandleReceivedCanMessage(uint8_t logIndex, const CanImuData6Axis& data);
    void HandleReceivedCanMessage(uint8_t logIndex, const CanAccelData& data);
    void HandleReceivedCanMessage(uint8_t logIndex, const CanGyroData& data);
    void HandleReceivedCanMessage(uint8_t logIndex, const CanMagData& data);

    template <typename T>
    bool ReceiveByIndex(uint8_t logIndex);

    CanAutoNodeDaughter *daughter = nullptr;
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* CANBUSTASK_HPP_ */
