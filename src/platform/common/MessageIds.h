/******************************************************************************
*
* File name:   MessageId.h
* Subsystem:   Platform Services
* Description: Contains system wide message Id declarations
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_ID_H_
#define _PLAT_MESSAGE_ID_H_

/**
 * 16 bit message ids are made up of:
 *     MSB 6 bits  - module id
 *     LSB 10 bits - message id within that module
 * The following masks aid in finding a module id and a actual message id
 * given a 16 bit message id.
 */

#define MESSAGE_MODULE_MASK          0xFC00
#define MESSAGE_ID_MASK              0x03FF

enum MessageId 
{
   // Base Message Id 
   MESSAGE_BASE_ID                             = 0x0000,  //Module id 0

   // Platform components use the next 4 bits to identify a subcomponent
   // within the system - this leaves only 6 bits (= 64 messages) for message ids
   // under each sub component

   // MsgMgr message ids
   MSGMGR_BASE = MESSAGE_BASE_ID,              // Platform Subcomponent 0
      MSGMGR_BASE_ID                           = 0x0001,
      MSGMGR_BASE_TIMER_ID                     = 0x0002,
      MSGMGR_DISCOVERY_MSG_ID                  = 0x0003,
      MSGMGR_DISCOVERY_LOCAL_MSG_ID            = 0x0004,
      MSGMGR_TEST1_MSG_ID                      = 0x0005,
      MSGMGR_TEST2_MSG_ID                      = 0x0006,
      MSGMGR_TEST_TIMER_MSG_ID                 = 0x0007,
      MSGMGR_TEST_DISTRIBUTED_MSG_ID           = 0x0008,
      MSGMGR_TEST_GROUP_MSG_ID                 = 0x0009,

   // Process Manager related messages
   PROCMGR_BASE = 0x0400,                      // Platform Subcomponent 1
      PROCMGR_HEART_BEAT_MSG_ID                = 0x0401,
      PROCMGR_HEART_BEAT_RESPONSE_MSG_ID       = 0x0402,

   // Timer Manager related messages
   TIMERMGR_BASE = 0x0800,                     // Platform Subcomponent 2

   RESOURCE_MONITOR_BASE = 0x0900,             // Platform Subcomponent 3
      RESOURCE_MONITOR_TIMER_MSG_ID            = 0x0901,

   FAULT_MANAGER_BASE = 0x0A00,                // Platform Subcomponent 4
      FAULT_MANAGER_ALARM_EVENT_MSG_ID         = 0x0A01,


   MESSAGE_LAST
};
#endif
