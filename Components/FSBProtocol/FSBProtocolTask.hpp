/**
 ********************************************************************************
 * @file    FSBProtocolTask.hpp
 * @brief   FSB-specific SOAR protobuf protocol task.
 ********************************************************************************
 */

#ifndef COMPONENTS_FSBPROTOCOL_FSBPROTOCOLTASK_HPP_
#define COMPONENTS_FSBPROTOCOL_FSBPROTOCOLTASK_HPP_

#include "ProtocolTask.hpp"
#include "FSBProtocolTypes.hpp"

class FSBProtocolTask : public ProtocolTask
{
public:
    static FSBProtocolTask& Inst()
    {
        static FSBProtocolTask inst;
        return inst;
    }

    void InitTask() override;

protected:
    static void RunTask(void* pvParams) { FSBProtocolTask::Inst().Run(pvParams); }

    void HandleProtobufCommandMessage(
        EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer) override;
    void HandleProtobufControlMesssage(
        EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer) override;
    void HandleProtobufTelemetryMessage(
        EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer) override;

private:
    FSBProtocolTask();
    FSBProtocolTask(const FSBProtocolTask&);
    FSBProtocolTask& operator=(const FSBProtocolTask&);
};

#endif /* COMPONENTS_FSBPROTOCOL_FSBPROTOCOLTASK_HPP_ */
