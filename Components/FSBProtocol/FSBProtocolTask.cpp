/**
 ********************************************************************************
 * @file    FSBProtocolTask.cpp
 * @brief   FSB-specific SOAR protobuf protocol task.
 ********************************************************************************
 */

#include "FSBProtocolTask.hpp"

#include "CommandCenterTask.hpp"
#include "UARTTask.hpp"

FSBProtocolTask::FSBProtocolTask()
    : ProtocolTask(Proto::Node::NODE_FSB, UART::RADIO, UART_TASK_COMMAND_SEND_RADIO),
      nextTxSequenceNum(1)
{
}

void FSBProtocolTask::InitTask()
{
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize FSBProtocolTask twice");

    BaseType_t rtValue = xTaskCreate(
        (TaskFunction_t)FSBProtocolTask::RunTask,
        (const char *)"FSBProtocolTask",
        (uint16_t)TASK_PROTOCOL_STACK_DEPTH_WORDS,
        (void *)this,
        (UBaseType_t)TASK_PROTOCOL_PRIORITY,
        (TaskHandle_t *)&rtTaskHandle);

    SOAR_ASSERT(rtValue == pdPASS, "FSBProtocolTask::InitTask() - xTaskCreate() failed");
}

void FSBProtocolTask::HandleProtobufCommandMessage(
    EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer)
{
    Proto::CommandMessage msg;
    const EmbeddedProto::Error err = msg.deserialize(readBuffer);

    if (EmbeddedProto::Error::NO_ERRORS != err)
    {
        SOAR_PRINT("FSBProtocolTask: failed to deserialize command protobuf [%d]\n",
                   static_cast<uint32_t>(err));
        SendNACK(Proto::MessageID::MSG_COMMAND, Proto::Node::NODE_UNKNOWN);
        return;
    }

    FSBProtoCommandRoute route = {};
    route.source = msg.get_source();
    route.target = msg.get_target();
    route.sourceSequenceNum = msg.get_source_sequence_num();
    route.messageType = msg.get_which_message();
    route.commandEnum = 0;
    route.commandParam = 0;

    switch (route.messageType)
    {
    case Proto::CommandMessage::FieldNumber::FCB_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_fcb_command().get_command_enum());
        break;
    case Proto::CommandMessage::FieldNumber::PBB_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_pbb_command().get_command_enum());
        break;
    case Proto::CommandMessage::FieldNumber::FSB_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_fsb_command().get_command_enum());
        route.commandParam = msg.get_fsb_command().get_command_param();
        break;
    case Proto::CommandMessage::FieldNumber::LRB_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_lrb_command().get_command_enum());
        route.commandParam = msg.get_lrb_command().get_command_param();
        break;
    default:
        SOAR_PRINT("FSBProtocolTask: command protobuf has no command payload\n");
        SendNACK(Proto::MessageID::MSG_COMMAND, msg.get_source());
        return;
    }

    Command cm(TASK_SPECIFIC_COMMAND, COMMAND_CENTER_ROUTE_PROTO_COMMAND);
    if (!cm.CopyDataToCommand(reinterpret_cast<uint8_t *>(&route), sizeof(route)))
    {
        SOAR_PRINT("FSBProtocolTask: failed to allocate routed command\n");
        SendNACK(Proto::MessageID::MSG_COMMAND, msg.get_source());
        return;
    }

    if (!CommandCenterTask::Inst().GetEventQueue()->Send(cm))
    {
        cm.Reset();
        SOAR_PRINT("FSBProtocolTask: failed to queue routed command\n");
        SendNACK(Proto::MessageID::MSG_COMMAND, msg.get_source());
    }
}

bool FSBProtocolTask::SendFcbRadioCommand(Proto::FcbCommand::Command command, uint32_t sequenceNum)
{
    Proto::CommandMessage msg;
    msg.set_source(Proto::Node::NODE_FSB);
    msg.set_target(Proto::Node::NODE_FCB);
    msg.set_source_sequence_num(sequenceNum == 0 ? nextTxSequenceNum++ : sequenceNum);
    msg.mutable_fcb_command().set_command_enum(command);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    const EmbeddedProto::Error err = msg.serialize(writeBuffer);
    if (EmbeddedProto::Error::NO_ERRORS != err)
    {
        SOAR_PRINT("FSBProtocolTask: failed to serialize FCB command [%d]\n",
                   static_cast<uint32_t>(err));
        return false;
    }

    SOAR_PRINT("FSBProtocolTask: sending FCB command %lu over radio seq=%lu\n",
               static_cast<uint32_t>(command),
               msg.get_source_sequence_num());
    SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_COMMAND);
    return true;
}

void FSBProtocolTask::HandleProtobufControlMesssage(
    EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer)
{
    (void)readBuffer;
    SOAR_PRINT("FSBProtocolTask: control messages are not routed yet\n");
}

void FSBProtocolTask::HandleProtobufTelemetryMessage(
    EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer)
{
    (void)readBuffer;
    SOAR_PRINT("FSBProtocolTask: telemetry messages are not routed yet\n");
}
