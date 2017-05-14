/******************************************************************************
*
* File name:   AlarmEventMessage.cpp
* Subsystem:   Platform Services
* Description: AlarmEventMessage is used for delivery of Alarm and Event Report
*              notifications to the EMS' Distributed Mailbox.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "AlarmEventMessage.h"

#include "platform/msgmgr/MessageBuffer.h"

#include "platform/common/MessageIds.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#define VERSION_NUMBER 1

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
AlarmEventMessage::AlarmEventMessage(const MailboxAddress& sourceAddress, AlarmEventMessageType messageType,
    string neid, unsigned short managedObject, unsigned int managedObjectInstance,
    unsigned short alarmCode, unsigned short eventCode, unsigned int pid, unsigned int timeStamp,
    unsigned int sourceContextId, unsigned int destinationContextId)
  :MessageBase(sourceAddress, VERSION_NUMBER, sourceContextId, destinationContextId),
   messageType_(messageType), neid_(neid), managedObject_(managedObject),
   managedObjectInstance_(managedObjectInstance), alarmCode_(alarmCode),
   eventCode_(eventCode), pid_(pid), timeStamp_(timeStamp)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
AlarmEventMessage::~AlarmEventMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Serialize this message into the supplied message buffer
// Design:
//-----------------------------------------------------------------------------
int AlarmEventMessage::serialize(MessageBuffer& buffer)
{
   // Perform the serialization
   // First, always serialize the source Address and Version!
   buffer << sourceAddress_;
   buffer << versionNumber_;

   // Optionally, serialize the source and destination context Ids (this can
   // be skipped if the application does not use them)
   buffer << sourceContextId_;
   buffer << destinationContextId_;

   // Now, serialize the message's custom/unique parameters   
   buffer << (int)messageType_;
   buffer << neid_;
   buffer << managedObject_;
   buffer << managedObjectInstance_;
   buffer << alarmCode_;
   buffer << eventCode_;
   buffer << pid_;
   buffer << timeStamp_;
   return OK;
}//end serialize


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Deserialize the supplied message buffer and return a Message ptr
// Design:
//-----------------------------------------------------------------------------
MessageBase* AlarmEventMessage::deserialize(MessageBuffer* buffer)
{
   MailboxAddress sourceAddress;
   unsigned int versionNumber;

   unsigned int sourceContextId;
   unsigned int destinationContextId;

   int messageType;
   string neid;
   unsigned short managedObject;
   unsigned int managedObjectInstance;
   unsigned short alarmCode;
   unsigned short eventCode;
   unsigned int pid;
   unsigned int timeStamp;

   // Perform the deserialization
   // First, always deserialize the sourceAddress and versionNumber
   *buffer >> sourceAddress;
   *buffer >> versionNumber;

   // Optionally deserialize the source and destination context Ids, if used
   *buffer >> sourceContextId;
   *buffer >> destinationContextId;

   // Deserialize the custom/unique message parameters
   *buffer >> messageType;
   *buffer >> neid;
   *buffer >> managedObject;
   *buffer >> managedObjectInstance;
   *buffer >> alarmCode;
   *buffer >> eventCode;
   *buffer >> pid;
   *buffer >> timeStamp;

   // Since this is not a poolable (OPM) message, just create one on the heap
   // which will be deleted by the MgrMgr framework
   AlarmEventMessage* remoteMessage = new AlarmEventMessage(sourceAddress, (AlarmEventMessageType)messageType,
      neid, managedObject, managedObjectInstance, alarmCode, eventCode, pid, timeStamp,
      sourceContextId, destinationContextId);
   return remoteMessage;
}//end deserialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Type
// Design:
//-----------------------------------------------------------------------------
AlarmEventMessageType AlarmEventMessage::getMessageType() const
{
   return messageType_;
}//end getMessageType


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the Alarm Code
// Design:
//-----------------------------------------------------------------------------
unsigned short AlarmEventMessage::getAlarmCode() const
{
   return alarmCode_;
}//end getAlarmCode


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the Event Code
// Design:
//-----------------------------------------------------------------------------
unsigned short AlarmEventMessage::getEventCode() const
{
   return eventCode_;
}//end getEventCode


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the NEID
// Design:
//-----------------------------------------------------------------------------
string AlarmEventMessage::getNEID() const
{
   return neid_;
}//end getNEID


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the Managed Object
// Design:
//-----------------------------------------------------------------------------
unsigned short AlarmEventMessage::getManagedObject() const
{
   return managedObject_;
}//end getManagedObject


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the Managed Object Instance
// Design:
//-----------------------------------------------------------------------------
unsigned int AlarmEventMessage::getManagedObjectInstance() const
{
   return managedObjectInstance_;
}//end getManagedObjectInstance


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the Process Id
// Design:
//-----------------------------------------------------------------------------
unsigned int AlarmEventMessage::getPid() const
{
   return pid_;
}//end getPid


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the Timestamp
// Design:
//-----------------------------------------------------------------------------
unsigned int AlarmEventMessage::getTimestamp() const
{
   return timeStamp_;
}//end getTimestamp


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Id
// Design:
//-----------------------------------------------------------------------------
unsigned short AlarmEventMessage::getMessageId() const
{
   return FAULT_MANAGER_ALARM_EVENT_MSG_ID;
}//end getMessageId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string AlarmEventMessage::toString()
{
   ostringstream ostr;
   ostr << "AlarmEventMessage: MessageType=" << messageType_
        << " NEID=" << neid_
        << " ManagedObject=" << managedObject_ 
        << " ManagedObjectInstance=" << managedObjectInstance_ 
        << " AlarmCode=" << alarmCode_
        << " EventCode=" << eventCode_
        << " Pid=" << pid_
        << " Timestamp=" << timeStamp_
        << " SourceAddress=" << sourceAddress_.toString();
   return (ostr.str());
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

