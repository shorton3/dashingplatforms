/******************************************************************************
*
* File name:   MessageTestGroupMessage.cpp
* Subsystem:   Platform Services
* Description: Test Message for Group Mailbox functionality 
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

#include "MessageTestGroupMessage.h"

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
MessageTestGroupMessage::MessageTestGroupMessage(const MailboxAddress& sourceAddress,
                                                   int ourIntValue,
                                                   string ourStringValue)
  :MessageBase(sourceAddress, VERSION_NUMBER),
   ourIntValue_(ourIntValue),
   ourStringValue_(ourStringValue)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTestGroupMessage::~MessageTestGroupMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Serialize this message into the supplied message buffer
// Design:
//-----------------------------------------------------------------------------
int MessageTestGroupMessage::serialize(MessageBuffer& buffer)
{
   // Perform the serialization
   buffer << sourceAddress_;
   buffer << ourIntValue_;
   buffer << ourStringValue_;
   return OK;
}//end serialize


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Deserialize the supplied message buffer and return a Message ptr
// Design:
//-----------------------------------------------------------------------------
MessageBase* MessageTestGroupMessage::deserialize(MessageBuffer* buffer)
{
   MailboxAddress sourceAddress;
   int ourIntValue;
   string ourStringValue;
                                                                                                           
   // Perform the deserialization
   *buffer >> sourceAddress;
   *buffer >> ourIntValue;
   *buffer >> ourStringValue;

   // Since this is not a poolable (OPM) message, just create one on the heap
   // which will be deleted by the MgrMgr framework
   MessageTestGroupMessage* remoteMessage = new MessageTestGroupMessage(sourceAddress, ourIntValue, ourStringValue);
   return remoteMessage;
}//end deserialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Id
// Design:
//-----------------------------------------------------------------------------
unsigned short MessageTestGroupMessage::getMessageId() const
{
   return MSGMGR_TEST_GROUP_MSG_ID;
}//end getMessageId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MessageTestGroupMessage::toString()
{
   ostringstream ostr;
   ostr << "MessageTestGroupMessage: OurIntValue=" << ourIntValue_ 
        << " OurStringValue=" << ourStringValue_ 
        << " Source Address=" << sourceAddress_.toString()
        << ends;
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

