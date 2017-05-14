/******************************************************************************
*
* File name:   DiscoveryLocalMessage.cpp
* Subsystem:   Platform Services
* Description: This class implements a local type message that is used for
*              sending local commands to the DiscoveryManager mailbox (for
*              registering/deregistering and displaying local entries)
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

#include "DiscoveryLocalMessage.h"

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
DiscoveryLocalMessage::DiscoveryLocalMessage(const MailboxAddress& sourceAddress,
   DiscoveryLocalOperationType operation, const MailboxAddress& discoveryAddress)
   : MessageBase(sourceAddress, VERSION_NUMBER),
     operation_(operation),
     discoveryAddress_(discoveryAddress)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Copy Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
DiscoveryLocalMessage::DiscoveryLocalMessage(const DiscoveryLocalMessage& rhs)
   : MessageBase(rhs.sourceAddress_, rhs.versionNumber_)
{
   operation_ = rhs.operation_;
   discoveryAddress_ = rhs.discoveryAddress_;
}//end copy constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DiscoveryLocalMessage::~DiscoveryLocalMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Id
// Design:
//-----------------------------------------------------------------------------
unsigned short DiscoveryLocalMessage::getMessageId() const
{
   return MSGMGR_DISCOVERY_LOCAL_MSG_ID;
}//end getMessageId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the Discovery Operation type
// Design:
//-----------------------------------------------------------------------------
DiscoveryLocalOperationType DiscoveryLocalMessage::getOperationType() const
{
   return operation_;
}//end getOperationType


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the Discovery Address which is being registered/deregistered
// Design:
//-----------------------------------------------------------------------------
const MailboxAddress& DiscoveryLocalMessage::getDiscoveryAddress() const
{
   return discoveryAddress_;
}//end getDiscoveryAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string DiscoveryLocalMessage::toString()
{
   ostringstream ostr;
   ostr << "DiscoveryLocalMessage: SourceAddress=" << sourceAddress_.toString();
   if (operation_ == DISCOVERY_LOCAL_REGISTER)
   {
      ostr << " Operation=REGISTER";
   }//end if
   else if (operation_ == DISCOVERY_LOCAL_DEREGISTER)
   {
      ostr << " Operation=DEREGISTER";
   }//end else if
   else if (operation_ == DISCOVERY_LOCAL_DISPLAY)
   {
      ostr << " Operation=DISPLAY";
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

