/**
 ********************************************************************************
 * @file    RadioTask.hpp
 * @author  spiro
 * @date    Apr 23, 2026
 * @brief   Task for controlling radio input
 ********************************************************************************
 */

#ifndef RADIO_RADIOTASK_HPP_
#define RADIO_RADIOTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "UARTDriver.hpp"
#include "DataBroker.hpp"
//#include "IMUTask.hpp"
//#include "LSM6DSOTask.hpp"
#include "mmc5983Task.hpp"
#include "BaroTask07.hpp"
#include "BaroTask11.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/
constexpr uint16_t RADIO_RX_BUFFER_SZ_BYTES = 128;
constexpr uint8_t TASK_RADIO_PRIORITY = 2;
constexpr uint8_t TASK_RADIO_QUEUE_DEPTH_OBJS = 10;
constexpr uint16_t TASK_RADIO_STACK_DEPTH_WORDS = 512;

/************************************
 * TYPEDEFS
 ************************************/
enum RADIO_TASK_COMMANDS {
  RADIO_TASK_COMMAND_NONE = 0,
  EVENT_RADIO_RX_COMPLETE
};

/************************************
 * CLASS DEFINITIONS
 ************************************/
class RadioTask : public Task, public UARTReceiverBase {
 public:
  static RadioTask& Inst() {
    static RadioTask inst;
    return inst;
  }

  void InitTask();

  // Interrupt receive callback
  void InterruptRxData(uint8_t errors);

  static bool radioEnabled;

 protected:
  static void RunTask(void* pvParams) {
    RadioTask::Inst().Run(pvParams);
  }

  void Run(void* pvParams);

  void ConfigureUART();
  void HandleRadioMessage(const char* msg);

  bool ReceiveData();

  // Helper functions
  static int32_t ExtractIntParameter(const char* msg, uint16_t identifierLen);

  // Member variables
  uint8_t radioBuffer[RADIO_RX_BUFFER_SZ_BYTES + 1];
  uint8_t radioMsgIdx;
  bool isRadioMsgReady;

  uint8_t radioRxChar;

  UARTDriver* const kUart_;

 private:
  RadioTask();
  RadioTask(const RadioTask&);
  RadioTask& operator=(const RadioTask&);
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* RADIO_RADIOTASK_HPP_ */
 
