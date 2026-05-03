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

#if __has_include("CanAutoNodeMotherboard.hpp") && __has_include("CanAutoNode.hpp")
#include "CanAutoNode.hpp"
#include "CanAutoNodeMotherboard.hpp"
#define COMMAND_CENTER_HAS_CAN_AUTO_NODE 1
#else
#define COMMAND_CENTER_HAS_CAN_AUTO_NODE 0
#endif

// TODO: Implement this into the new global canbus log index file in another submodule
#define COMMAND_CENTER_LOGGING_COMMAND_LOG_INDEX 0

enum CommandCenterCommands {
	START_LOGGING,
	STOP_LOGGING,
	SEND_FILES,
	COMMAND_CENTER_ROUTE_PROTO_COMMAND
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
#if COMMAND_CENTER_HAS_CAN_AUTO_NODE
     std::vector<CanAutoNode::UniqueBoardID> activeBoards;                          //list of all active daughterboards
     CanAutoNodeMotherboard* motherboard = nullptr;
#endif
 };
 
 /************************************
  * FUNCTION DECLARATIONS
  ************************************/
 
 #endif
