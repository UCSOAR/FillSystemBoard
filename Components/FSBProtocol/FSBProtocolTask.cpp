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
    : ProtocolTask(Proto::Node::NODE_UNKNOWN, UART::RADIO, UART_TASK_COMMAND_SEND_RADIO)
{
}

void FSBProtocolTask::InitTask()
{
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize FSBProtocolTask twice");

    BaseType_t rtValue = xTaskCreate(
        (TaskFunction_t)FSBProtocolTask::RunTask,
        (const char*)"FSBProtocolTask",
        (uint16_t)TASK_PROTOCOL_STACK_DEPTH_WORDS,
        (void*)this,
        (UBaseType_t)TASK_PROTOCOL_PRIORITY,
        (TaskHandle_t*)&rtTaskHandle);

    SOAR_ASSERT(rtValue == pdPASS, "FSBProtocolTask::InitTask() - xTaskCreate() failed");
}

void FSBProtocolTask::HandleProtobufCommandMessage(
    EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::CommandMessage msg;
    const EmbeddedProto::Error err = msg.deserialize(readBuffer);

    if (EmbeddedProto::Error::NO_ERRORS != err) {
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

    switch (route.messageType) {
    case Proto::CommandMessage::FieldNumber::DMB_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_dmb_command().get_command_enum());
        break;
    case Proto::CommandMessage::FieldNumber::PBB_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_pbb_command().get_command_enum());
        break;
    case Proto::CommandMessage::FieldNumber::RCU_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_rcu_command().get_command_enum());
        route.commandParam = msg.get_rcu_command().get_command_param();
        break;
    case Proto::CommandMessage::FieldNumber::SOB_COMMAND:
        route.commandEnum = static_cast<uint32_t>(msg.get_sob_command().get_command_enum());
        route.commandParam = msg.get_sob_command().get_command_param();
        break;
    default:
        SOAR_PRINT("FSBProtocolTask: command protobuf has no command payload\n");
        SendNACK(Proto::MessageID::MSG_COMMAND, msg.get_source());
        return;
    }

    Command cm(TASK_SPECIFIC_COMMAND, COMMAND_CENTER_ROUTE_PROTO_COMMAND);
    if (!cm.CopyDataToCommand(reinterpret_cast<uint8_t*>(&route), sizeof(route))) {
        SOAR_PRINT("FSBProtocolTask: failed to allocate routed command\n");
        SendNACK(Proto::MessageID::MSG_COMMAND, msg.get_source());
        return;
    }

    if (!CommandCenterTask::Inst().GetEventQueue()->Send(cm)) {
        cm.Reset();
        SOAR_PRINT("FSBProtocolTask: failed to queue routed command\n");
        SendNACK(Proto::MessageID::MSG_COMMAND, msg.get_source());
    }
}

void FSBProtocolTask::HandleProtobufControlMesssage(
    EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    (void)readBuffer;
    SOAR_PRINT("FSBProtocolTask: control messages are not routed yet\n");
}

void FSBProtocolTask::HandleProtobufTelemetryMessage(
    EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    (void)readBuffer;
    SOAR_PRINT("FSBProtocolTask: telemetry messages are not routed yet\n");
}
