/**
 ********************************************************************************
 * @file    FSBProtoTask.hpp
 * @author  Christy Guirguis
 * @date    May 12, 2026
 * @brief
 ********************************************************************************
 */

#ifndef FSBPROTOTASK_HPP_
#define FSBPROTOTASK_HPP_

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
class FSBProtocolTask : public ProtocolTask
{
public:
    static FSBProtocolTask &Inst()
    {
        static FSBProtocolTask inst;
        return inst;
    }

    void InitTask();

    static void SendProtobufMessage(EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> &writeBuffer, Proto::MessageID msgId)
    {
        Inst().ProtocolTask::SendProtobufMessage(writeBuffer, msgId);
    }

protected:
    static void RunTask(void *pvParams) { FSBProtocolTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    // These handlers will receive a buffer and size corresponding to a decoded message
    void HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer);
    void HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer);
    void HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer);

private:
    FSBProtocolTask();                                   // Private constructor
    FSBProtocolTask(const FSBProtocolTask &);            // Prevent copy-construction
    FSBProtocolTask &operator=(const FSBProtocolTask &); // Prevent assignment
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* FSBPROTOTASK_HPP_ */
