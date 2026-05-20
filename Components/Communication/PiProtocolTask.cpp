/**
 ********************************************************************************
 * @file    PiProtocolTask.cpp
 * @author  Christy
 * @date    May 19, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "PiProtocolTask.hpp"
#include "ReadBufferFixedSize.h"
#include "UARTTask.hpp"
#include "GPIO.hpp"
// #include "RCUProtoTask.hpp"
/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/

/**
 * @brief Initialize the PiProtocolTask
 */
void PiProtocolTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Protocol task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)PiProtocolTask::RunTask,
            (const char*)"ProtocolTask",
            (uint16_t)TASK_PROTOCOL_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_PROTOCOL_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeded
    SOAR_ASSERT(rtValue == pdPASS, "ProtocolTask::InitTask - xTaskCreate() failed");
}

/**
 * @brief Default constructor
 */
PiProtocolTask::PiProtocolTask() : ProtocolTask(Proto::Node::NODE_FSB,
    UART::RPI,
    UART_TASK_COMMAND_SEND_PI)
{
}

/**
 * @brief Handle a command message
 */
void PiProtocolTask::HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::CommandMessage msg;
    msg.deserialize(readBuffer);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    // //Send to relevant destination
    // if(msg.get_target() == Proto::Node::NODE_FCB) {
    //     DMBRxProtocolTask::Inst().SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_COMMAND);
    //     // TODO NEW MAKE SURE SAFE IN PBB
    //     RCUProtocolTask::Inst().SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_COMMAND);
    //     return;
    // }

    // if(msg.get_target() == Proto::Node::NODE_PBB) {
    //     PiProtocolTask::Inst().SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_COMMAND);
    //     return;
    // }

    // if(msg.get_target() != Proto::Node::NODE_FSB) {
    //     return;
    // }

    switch(msg.get_fsb_command().get_command_enum()) {
    case Proto::FsbCommand::Command::FSB_TARE_NOS1_LOAD_CELL: {
    	//NOTE: WORKS FOR TO NOS1 ONLY
        SOAR_PRINT("PROTO-INFO: Received FSB Tare NOS1 Load Cell Command\n");
        LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)NOS1_LOADCELL_REQUEST_TARE));
        break;
    }
    case Proto::FsbCommand::Command::FSB_TARE_NOS2_LOAD_CELL: {
    	//NOTE: WORKS FOR TO NOS1 ONLY
        SOAR_PRINT("PROTO-INFO: Received FSB Tare NOS2 Load Cell Command\n");
        LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)NOS2_LOADCELL_REQUEST_TARE));
        break;
    }
    case Proto::FsbCommand::Command::FSB_CALIBRATE_NOS1_LOAD_CELL: {
    	//NOTE: WORKS FOR TO NOS1 ONLY
        SOAR_PRINT("PROTO-INFO: Received FSB Calibrate NOS1 Load Cell Command\n");
        int32_t mass_mg = msg.get_fsb_command().get_command_param();
		LoadCellTask::Inst().SetCalibrationMassGrams((float)mass_mg / 1000);
		LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, NOS1_LOADCELL_REQUEST_CALIBRATE));
		break;
    }
    case Proto::FsbCommand::Command::FSB_CALIBRATE_NOS2_LOAD_CELL: {
    	//NOTE: WORKS FOR TO NOS1 ONLY
        SOAR_PRINT("PROTO-INFO: Received FSB Calibrate NOS2 Load Cell Command\n");
        int32_t mass_mg = msg.get_fsb_command().get_command_param();
		LoadCellTask::Inst().SetCalibrationMassGrams((float)mass_mg / 1000);
		LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, NOS2_LOADCELL_REQUEST_CALIBRATE));
		break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_AC1: {
        GPIO::SHEDAC::Off();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_AC1: {
        GPIO::SHEDAC::On();
        break;
    }
    case Proto::FsbCommand::Command::FSB_KILL_PAD_BOX1: {
        GPIO::PADBOX1::Kill();
        GPIO::PADBOX2::Kill();
        break;
    }
    case Proto::FsbCommand::Command::FSB_IGNITE_PAD_BOX1: {
        GPIO::PADBOX1::Ignite();
        GPIO::PADBOX2::Ignite();
        break;
    }
    case Proto::FsbCommand::Command::FSB_KILL_PAD_BOX2: {
        GPIO::PADBOX1::Kill();
        GPIO::PADBOX2::Kill();
        break;
    }
    case Proto::FsbCommand::Command::FSB_IGNITE_PAD_BOX2: {
        GPIO::PADBOX1::Ignite();
        GPIO::PADBOX2::Ignite();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_PBV1: {
        GPIO::PBV1::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_PBV1: {
        GPIO::PBV1::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_PBV2: {
        GPIO::PBV2::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_PBV2: {
        GPIO::PBV2::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_PBV3: {
        GPIO::PBV3::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_PBV3: {
        GPIO::PBV3::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_PBV4: {
        GPIO::PBV4::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_PBV4: {
        GPIO::PBV4::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_SOL5: {
        GPIO::SOL5::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_SOL5: {
        GPIO::SOL5::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_SOL6: {
        GPIO::SOL6::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_SOL6: {
        GPIO::SOL6::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_SOL7: {
        GPIO::SOL7::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_SOL7: {
        GPIO::SOL7::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_SOL8A: {
        GPIO::SOL8A::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_SOL8A: {
        GPIO::SOL8A::Close();
        break;
    }
    case Proto::FsbCommand::Command::FSB_OPEN_SOL8B: {
        GPIO::SOL8B::Open();
        break;
    }
    case Proto::FsbCommand::Command::FSB_CLOSE_SOL8B: {
        GPIO::SOL8B::Close();
        break;
    }
    default:
        SOAR_PRINT("PiProtocolTask - Received Unsupported FSB commmand {%d}\n", msg.get_fsb_command());
		break;
    }

}

/**
 * @brief Handle a control message
 */
void PiProtocolTask::HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    //rewrap into a write buffer var because readBuffer and writeBuffer are not interchangeable
    Proto::TelemetryMessage msg;
    msg.deserialize(readBuffer);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    // Send to relevant destination
    // if(msg.get_target() == Proto::Node::NODE_FCB || msg.get_target() == Proto::Node::NODE_PBB) {
    //     DMBRxProtocolTask::Inst().SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_CONTROL);
    //     return;
    // }

    // if(msg.get_target() == Proto::Node::NODE_SOB) {
    //     SOBRxRepeaterTask::Inst().SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_CONTROL);
    //     return;
    // }
}

/**
 * @brief Handle a telemetry message (unused?)
 */
void PiProtocolTask::HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{

}
