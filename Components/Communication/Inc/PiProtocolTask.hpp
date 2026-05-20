/**
 ********************************************************************************
 * @file    PiProtocolTask.hpp
 * @author  Christy
 * @date    May 19, 2026
 * @brief
 ********************************************************************************
 */

#ifndef COMMUNICATION_INC_PIPROTOCOLTASK_HPP_
#define COMMUNICATION_INC_PIPROTOCOLTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "ProtocolTask.hpp"
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "UARTTask.hpp"
/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/
class PiProtocolTask : public ProtocolTask
{
public:
    static PiProtocolTask& Inst() {
        static PiProtocolTask inst;
        return inst;
    }

    void InitTask();

    static void SendProtobufMessage(EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE>& writeBuffer, Proto::MessageID msgId)
    {
        Inst().ProtocolTask::SendProtobufMessage(writeBuffer, msgId);
    }

protected:
    static void RunTask(void* pvParams) { PiProtocolTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    // These handlers will receive a buffer and size corresponding to a decoded message
    void HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    
    // Member variables

private:
    PiProtocolTask();        // Private constructor
    PiProtocolTask(const PiProtocolTask&);                        // Prevent copy-construction
    PiProtocolTask& operator=(const PiProtocolTask&);            // Prevent assignment
};

#endif /* COMMUNICATION_INC_PIPROTOCOLTASK_HPP_ */
