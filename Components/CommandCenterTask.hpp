/**
 ********************************************************************************
 * @file    ${file_name}
 * @author  ${user}
 * @date    ${date}
 * @brief   This is a template header file to create a new task in our firmware
 * 
 * Setup Steps
 * 1. Define the Task Queue Depth in SystemDefines.hpp
 * 2. Define the Task Stack Depth in SystemDefines.hpp
 * 3. Define the Task Priority in SystemDefines.hpp
 * 4. Replace all placeholders marked with a $ sign
 ********************************************************************************
 */

 #ifndef CCENTER_TASK_HPP_
 #define CCENTER_TASK_HPP_
 
 /************************************
  * INCLUDES
  ************************************/
 #include "Task.hpp"
 #include "SystemDefines.hpp"
#include <vector>
#include "CubeDefines.hpp"
#include "CanAutoNodeMotherboard.hpp"

// TODO: Implement this into the new global canbus log index file in another submodule
#define COMMAND_CENTER_LOGGING_COMMAND_LOG_INDEX 0

enum CommandCenterCommands {
	START_LOGGING,
	STOP_LOGGING,
	SEND_FILES
};


 
 /************************************
  * MACROS AND DEFINES
  ************************************/
 
 /************************************
  * TYPEDEFS
  ************************************/
 
 /************************************
  * CLASS DEFINITIONS
  ************************************/
 class CommandCenterTask : public Task
 {
 public:
     static CommandCenterTask& Inst() {
         static CommandCenterTask inst;
         return inst;
     }
 
     void InitTask();
 
 protected:
     static void RunTask(void* pvParams) { CommandCenterTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
     void Run(void * pvParams); // Main run code
     void HandleCommand(Command& cm);
 
 private:
     // Private Functions
     CommandCenterTask();        // Private constructor
     CommandCenterTask(const CommandCenterTask&);                        // Prevent copy-construction
     CommandCenterTask& operator=(const CommandCenterTask&);            // Prevent assignment
     void ExecuteCommand(const char* msg);
     std::vector<CanAutoNode::UniqueBoardID> activeBoards;                          //list of all active daughterboards
     CanAutoNodeMotherboard* motherboard = nullptr;
 };
 
 /************************************
  * FUNCTION DECLARATIONS
  ************************************/
 
 #endif
