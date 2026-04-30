/**
 ********************************************************************************
 * @file    ${file_name}
 * @author  ${user}
 * @date    ${date}
 * @brief   This is a template source file to create a new task in our firmware
 * 
 * Setup Steps
 * 1. Define the Task Queue Depth in SystemDefines.hpp
 * 2. Define the Task Stack Depth in SystemDefines.hpp
 * 3. Define the Task Priority in SystemDefines.hpp
 * 4. Replace all placeholders marked with a $ sign
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "CommandCenterTask.hpp"
#include "SystemDefines.hpp"
#include "DebugTask.hpp"
#include "FSBProtocol/FSBProtocolTypes.hpp"
#include <string>
#include <algorithm>
#include <vector>


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
 * @brief Constructor for CommandCenterTask
 */
CommandCenterTask::CommandCenterTask()
: Task(TASK_COMMANDCENTER_QUEUE_DEPTH_OBJS)
{
	//activeBoards = CanAutoNode::GetNamesOfAllBoards();
}

#if COMMAND_CENTER_HAS_CAN_AUTO_NODE
extern FDCAN_HandleTypeDef hfdcan1;
#endif
/**
 * @brief Initialize the CommandCenterTask
 *        Do not modify this function aside from adding the task name
 */
void CommandCenterTask::InitTask()
{
	// Make sure the task is not already initialized
	SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

	BaseType_t rtValue =
			xTaskCreate(
					(TaskFunction_t)CommandCenterTask::RunTask,
					"CommandCenterTask",
					(uint16_t)TASK_COMMANDCENTER_STACK_DEPTH_WORDS,
					(void*)this,
					(UBaseType_t)TASK_COMMANDCENTER_PRIORITY,
					(TaskHandle_t*)&rtTaskHandle
			);

	SOAR_ASSERT(rtValue == pdPASS, "CommandCenterTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void CommandCenterTask::Run(void * pvParams)
{

#if COMMAND_CENTER_HAS_CAN_AUTO_NODE
	motherboard = new CanAutoNodeMotherboard{&hfdcan1};
#endif
	while (1) {
		// Process commands in blocking mode
		Command cm;
		bool res = qEvtQueue->ReceiveWait(cm);
		if(res) {
			HandleCommand(cm);
		}
	}
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
//handles the command and decides to process or ignore
void CommandCenterTask::HandleCommand(Command& cm)
{
	switch (cm.GetCommand()) {
	case DATA_COMMAND:
		// Process the command
		if (cm.GetTaskCommand() == EVENT_DEBUG_RX_COMPLETE)
		{
			ExecuteCommand((const char*)cm.GetDataPointer());
		}
		break;
	case TASK_SPECIFIC_COMMAND:
		if (cm.GetTaskCommand() == COMMAND_CENTER_ROUTE_PROTO_COMMAND &&
			cm.GetDataSize() == sizeof(FSBProtoCommandRoute))
		{
			const FSBProtoCommandRoute* route =
				reinterpret_cast<const FSBProtoCommandRoute*>(cm.GetDataPointer());

			SOAR_PRINT("CommandCenterTask - Proto route src=%lu target=%lu type=%lu cmd=%lu param=%ld seq=%lu\n",
				static_cast<uint32_t>(route->source),
				static_cast<uint32_t>(route->target),
				static_cast<uint32_t>(route->messageType),
				route->commandEnum,
				route->commandParam,
				route->sourceSequenceNum);

			// TODO: Map route->target to the matching daughter board and forward over CAN.
			// For NODE_ANY, broadcast to every discovered daughter board.
		}
		break;

	default:
		SOAR_PRINT("CommandCenterTask - Received Unsupported Command {%d}\n", cm.GetCommand());
		break;
	}

	//No matter what we happens, we must reset data
	cm.Reset();
}

void CommandCenterTask::ExecuteCommand(const char* msg)
{

#if COMMAND_CENTER_HAS_CAN_AUTO_NODE
	//discover daughterboards while the task is running
	if(motherboard == nullptr) {
		return;
	}
	CanAutoNode::UniqueBoardID daus[16];
	motherboard->GetIDsOfAllBoards(daus,sizeof(daus)/sizeof(CanAutoNode::UniqueBoardID));
	activeBoards.assign(daus,daus+sizeof(daus)/sizeof(CanAutoNode::UniqueBoardID));

	//parse string to check if there is a start or an end
	std::string command(msg);

	//find which daughterboards are chosen
	std::vector<CanAutoNode::UniqueBoardID> daughterBoards;

	// start command
	if (command.find("start") == 0) {

		//if all appears all boards get put through commands
		if (command.find("all") != std::string::npos) {
			for (auto& board : activeBoards) {
				daughterBoards.push_back(board);
			}
		} else {
			//if d2, d3 etc gets typed in the commands get sent for those boards
			char names[MAX_NAME_STR_LEN][MAX_NAME_STR_LEN];
			uint16_t count = motherboard->GetNamesOfAllBoards(names, MAX_NAME_STR_LEN);
			for (uint16_t i = 0; i < count; i++) {
				const char* boardName = names[i];
				if (command.find(boardName) != std::string::npos) {
					daughterBoards.push_back(motherboard->GetIDOfBoardWithName(boardName));
				}
			}
		}


		if (daughterBoards.empty()) {
			SOAR_PRINT("Error: enter board names. e.g. DAQ, FSB, etc\n");
			return;
		}

		//Loop through boards
		for (auto& board : daughterBoards) {

			uint8_t cmd = CommandCenterCommands::START_LOGGING;
			motherboard->SendMessageToDaughterByLogIndex(board, COMMAND_CENTER_LOGGING_COMMAND_LOG_INDEX, &cmd);

		}

		//checking state of daughter boards and looping back
		bool ContinueLogging = true;
		while (ContinueLogging) {
			for (auto& board : daughterBoards) {
				//sends a can message
				uint8_t cmd = CommandCenterCommands::START_LOGGING;
				motherboard->SendMessageToDaughterByLogIndex(board, COMMAND_CENTER_LOGGING_COMMAND_LOG_INDEX, &cmd);

			}

			vTaskDelay(pdMS_TO_TICKS(1000)); //wait 1 second between the next check
		}
	}


	//       end command
	else if (command.find("end") == 0) {

		//if all appears all boards get put through commands
		if (command.find("all") != std::string::npos) {
			for (auto& board : activeBoards) {
				daughterBoards.push_back(board);
			}
		} else {

			for (size_t i = 0; i < activeBoards.size(); i++) {

				char names[MAX_NAME_STR_LEN][MAX_NAME_STR_LEN];
				uint16_t count = motherboard->GetNamesOfAllBoards(names, sizeof(names)/sizeof(names[0]));
				for (uint16_t i = 0; i < count; i++) {
					const char* boardName = names[i];
					if (command.find(boardName) != std::string::npos) {
						daughterBoards.push_back(motherboard->GetIDOfBoardWithName(boardName));
					}
				}
			}
		}

		if (daughterBoards.empty()) {
			SOAR_PRINT("Error: enter board names. e.g. DAQ, FSB, etc\n");
			return;
		}

		//stops logging and sends files
		for (auto& board : daughterBoards) {
			uint8_t cmd = STOP_LOGGING;
			motherboard->SendMessageToDaughterByLogIndex(board, COMMAND_CENTER_LOGGING_COMMAND_LOG_INDEX, &cmd);

		}

		//receive files
		for (auto& board : daughterBoards) {
			uint8_t cmd = SEND_FILES;

			if (motherboard->SendMessageToDaughterByLogIndex(board, COMMAND_CENTER_LOGGING_COMMAND_LOG_INDEX, &cmd)) {
				uint8_t out[64];
				while(motherboard->ReadMessageFromDaughterByLogIndex(board, COMMAND_CENTER_LOGGING_COMMAND_LOG_INDEX+1,out , sizeof(out))) {

				}
			}

		}

	}
#else
	(void)msg;
	SOAR_PRINT("CommandCenterTask - CAN auto-node routing is unavailable in this checkout\n");
#endif
}
