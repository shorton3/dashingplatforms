/******************************************************************************
*
* File name:   DiscoveryMessage.cpp
* Subsystem:   Platform Services
* Description: This class implements a remote/distributed type message that
*              communicates MailboxLookupService updates across remote nodes.
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

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DiscoveryMessage.h"

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
DiscoveryMessage::DiscoveryMessage(const MailboxAddress& sourceAddress,
   DiscoveryOperationType operation, unsigned int originatingPID, const MailboxAddress& discoveryAddress)
   : MessageBase(sourceAddress, VERSION_NUMBER),
     operation_(operation),
     discoveryAddress_(discoveryAddress),
     originatingPID_(originatingPID)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Copy Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
DiscoveryMessage::DiscoveryMessage(const DiscoveryMessage& rhs)
   : MessageBase(rhs.sourceAddress_, rhs.versionNumber_)
{
   operation_ = rhs.operation_;
   originatingPID_ = rhs.originatingPID_;
   discoveryAddress_ = rhs.discoveryAddress_;
}//end copy constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DiscoveryMessage::~DiscoveryMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Serialize this message into the supplied message buffer
// Design:
//-----------------------------------------------------------------------------
int DiscoveryMessage::serialize(MessageBuffer& buffer)
{
   // Perform the serialization
   buffer << sourceAddress_;
   buffer << (unsigned short)operation_;
   buffer << originatingPID_;
   buffer << discoveryAddress_;
   return OK;
}//end serialize


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Deserialize the supplied message buffer and return a Message ptr
// Design:
//-----------------------------------------------------------------------------
MessageBase* DiscoveryMessage::deserialize(MessageBuffer* buffer)
{
   MailboxAddress sourceAddress;
   unsigned short operation;
   unsigned int originatingPID;
   MailboxAddress discoveryAddress;

   // Perform the deserialization
   *buffer >> sourceAddress;
   *buffer >> operation;
   *buffer >> originatingPID;
   *buffer >> discoveryAddress;

   // Since this is not a poolable (OPM) message, just create one on the heap
   // which will be deleted by the MgrMgr framework
   DiscoveryMessage* discoveryMessage = new DiscoveryMessage(sourceAddress,
      (DiscoveryOperationType)operation, originatingPID, discoveryAddress);
   return discoveryMessage;
}//end deserialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Id
// Design:
//-----------------------------------------------------------------------------
unsigned short DiscoveryMessage::getMessageId() const
{
   return MSGMGR_DISCOVERY_MSG_ID;
}//end getMessageId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the Discovery Operation type
// Design:
//-----------------------------------------------------------------------------
DiscoveryOperationType DiscoveryMessage::getOperationType() const
{
   return operation_;
}//end getOperationType


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the Discovery Address which is being registered/deregistered
// Design:
//-----------------------------------------------------------------------------
const MailboxAddress& DiscoveryMessage::getDiscoveryAddress() const
{
   return discoveryAddress_;
}//end getDiscoveryAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the PID of the process that originated this message.
// Design: Used to verify and filter out messages from our own DiscoveryManager
//-----------------------------------------------------------------------------
unsigned int DiscoveryMessage::getOriginatingPID() const
{
   return originatingPID_;
}//end getOriginatingPID


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string DiscoveryMessage::toString()
{
   ostringstream ostr;
   ostr << "DiscoveryMessage: SourceAddress=" << sourceAddress_.toString();
   if (operation_ == DISCOVERY_REGISTER)
   {
      ostr << " Operation=REGISTER";
   }//end if
   else if (operation_ == DISCOVERY_DEREGISTER)
   {
      ostr << " Operation=DEREGISTER";
   }//end else if
   else
   {
      ostr << " Operation=Unknown(" << operation_ << ")";
   }//end 
   ostr << " DiscoveryAddress=" << discoveryAddress_.toString() << ends;
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

