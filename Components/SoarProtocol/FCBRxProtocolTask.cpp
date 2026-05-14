/**
 ******************************************************************************
 * File Name          : FCBRxProtocolTask.hpp
 * Description        : Protocol task, specific to FCBRx UART Line
 ******************************************************************************
 */
#include "FCBRxProtocolTask.hpp"
#include "FlightTask.hpp"
#include "ReadBufferFixedSize.h"
#include "PIRxProtocolTask.hpp"
#include "LRBRxRepeaterTask.hpp"
#include "UARTTask.hpp"

/**
 * @brief Initialize the FCBRxProtocolTask
 */
void FCBRxProtocolTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Protocol task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)FCBRxProtocolTask::RunTask,
                    (const char *)"ProtocolTask",
                    (uint16_t)TASK_PROTOCOL_STACK_DEPTH_WORDS,
                    (void *)this,
                    (UBaseType_t)TASK_PROTOCOL_PRIORITY,
                    (TaskHandle_t *)&rtTaskHandle);

    // Ensure creation succeded
    SOAR_ASSERT(rtValue == pdPASS, "ProtocolTask::InitTask - xTaskCreate() failed");
}

/**
 * @brief Default constructor
 */
FCBRxProtocolTask::FCBRxProtocolTask() : ProtocolTask(Proto::Node::NODE_FSB,
                                                      UART::Radio,
                                                      UART_TASK_COMMAND_SEND_FCB)
{
}

/**
 * @brief Handle a command message
 */
void FCBRxProtocolTask::HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer)
{
    Proto::CommandMessage msg;
    msg.deserialize(readBuffer);

    // Verify the source and target nodes, echo it if it does not have FCBRx as the target
    if (msg.get_source() != Proto::Node::NODE_FCB || msg.get_target() != Proto::Node::NODE_PBB)
        return;

    // If the message does not have a LRB command, do nothing
    if (!msg.has_pbb_command())
        return;

    SOAR_PRINT("PROTO-INFO: Received FCBRx Command Message");

    // Send LRB command
    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    // LRBRxRepeaterTask::Inst().SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_COMMAND);
}

/**
 * @brief Handle a control message
 */
void FCBRxProtocolTask::HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer)
{
    // rewrap into a write buffer var because readBuffer and writeBuffer are not interchangeable
    Proto::ControlMessage msg;
    msg.deserialize(readBuffer);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    PIRxProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_CONTROL);
}

/**
 * @brief Handle a telemetry message
 */
void FCBRxProtocolTask::HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES> &readBuffer)
{
    // rewrap into a write buffer var because readBuffer and writeBuffer are not interchangeable
    Proto::TelemetryMessage msg;
    msg.deserialize(readBuffer);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    PIRxProtocolTask::Inst().SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}
