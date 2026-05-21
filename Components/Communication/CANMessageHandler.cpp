/**
 ********************************************************************************
 * @file    CANMessageHandler.cpp
 * @author  Shivam Desai
 * @date    May 2, 2026
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "CANTask.hpp"

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
void CANTask::HandleIncomingCANMessages() {
	GSE_AC1_OPEN();
}

void CANTask::GSE_AC1_OPEN() {
	bool isDataAvailable = false;

	GSE_AC1_OPEN_COMMAND ac1OpenCommand{false};
	isDataAvailable = fcbCAN.ReadMessageFromDaughterByLogIndex(
		fcbCAN.GetIDOfBoardWithName(CAN_ROCKET_TARGET_DAQ),
		GSE_LogIndexes::_GSE_AC1_OPEN_COMMAND_LOGINDEX,
		(uint8_t*)&ac1OpenCommand,
		sizeof(GSE_AC1_OPEN_COMMAND)
	);

	if (isDataAvailable) {
		if (ac1OpenCommand.open) {
			SOAR_PRINT("AC1 Open Instruction received true");
		} else {
			SOAR_PRINT("AC1 Open Instruction received false");
		}
	}
}
