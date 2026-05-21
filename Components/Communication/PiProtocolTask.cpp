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
#include "CANTask.hpp"

// #include "GPIO.hpp"
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
    &Driver::usart1,   // placeholder idk which uart/if it needs a new one
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

    if(msg.get_target() != Proto::Node::NODE_FSB) {
        return;
    }

    switch(msg.get_fsb_command().get_command_enum()) {
    // case Proto::FsbCommand::Command::FSB_TARE_NOS1_LOAD_CELL: {
    // 	//NOTE: WORKS FOR TO NOS1 ONLY
    //     SOAR_PRINT("PROTO-INFO: Received FSB Tare NOS1 Load Cell Command\n");
    //     LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)NOS1_LOADCELL_REQUEST_TARE));
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_TARE_NOS2_LOAD_CELL: {
    // 	//NOTE: WORKS FOR TO NOS1 ONLY
    //     SOAR_PRINT("PROTO-INFO: Received FSB Tare NOS2 Load Cell Command\n");
    //     LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)NOS2_LOADCELL_REQUEST_TARE));
    //     break;
    // }
    case Proto::FsbCommand::Command::FSB_CALIBRATE_NOS1_LOAD_CELL: {
    	//NOTE: WORKS FOR TO NOS1 ONLY
        SOAR_PRINT("PROTO-INFO: Received FSB Calibrate NOS1 Load Cell Command\n");
        int32_t mass_mg = msg.get_fsb_command().get_command_param();
	// 	LoadCellTask::Inst().SetCalibrationMassGrams((float)mass_mg / 1000);
	// 	LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, NOS1_LOADCELL_REQUEST_CALIBRATE));
		break;
    }
    case Proto::FsbCommand::Command::FSB_CALIBRATE_NOS2_LOAD_CELL: {
    	//NOTE: WORKS FOR TO NOS1 ONLY
        SOAR_PRINT("PROTO-INFO: Received FSB Calibrate NOS2 Load Cell Command\n");
        int32_t mass_mg = msg.get_fsb_command().get_command_param();
		// LoadCellTask::Inst().SetCalibrationMassGrams((float)mass_mg / 1000);
		// LoadCellTask::Inst().SendCommand(Command(REQUEST_COMMAND, NOS2_LOADCELL_REQUEST_CALIBRATE));
		break;
    }






    case Proto::FsbCommand::Command::FSB_OPEN_AC1: {
        // GSE_AC1_OPEN_COMMAND ac1_command{true};
		// CANTask::Inst().SendCANMessageToDaughter(
		// 	CAN_GSE_TARGET_SOL1,
		// 	GSE_LogIndexes::_RPB_AIR_BRAKES_COMMAND_LOGINDEX,
		// 	(uint8_t*) &ac1_command
		// );





        GSE_AC1_OPEN_COMMAND ac1_command{false};
		CANTask::Inst().SendCANMessageToDaughter(
			CAN_GSE_TARGET_SOL1,
			GSE_LogIndexes::_GSE_AC1_OPEN_COMMAND_LOGINDEX,
			(uint8_t*) &ac1_command
		);
    }






    // case Proto::FsbCommand::Command::FSB_CLOSE_AC1: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_1;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_KILL_PAD_BOX1: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_1;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_IGNITE_PAD_BOX1: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_IGNITE;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_KILL_PAD_BOX2: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_1;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_IGNITE_PAD_BOX2: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_1;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_PBV1: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_PBV1: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_PBV2: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_PBV2: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_PBV3: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_PBV3: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_PBV4: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_PBV4: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_SOL5: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_SOL5: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_SOL6: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_SOL6: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_2;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_SOL7: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_3;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_SOL7: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_3;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_SOL8A: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_3;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_SOL8A: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_3;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_OPEN_SOL8B: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_3;
    //     Command cmd.enabled = 1;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
    // case Proto::FsbCommand::Command::FSB_CLOSE_SOL8B: {
    //     can msg;
    //     msg.target = Proto::Node::NODE_FSB_SOL_3;
    //     Command cmd.enabled = 0;
    //     msg.command = Proto::MessageID::MSG_CONTROL;
    //     sendCan(msg);
    //     break;
    // }
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
