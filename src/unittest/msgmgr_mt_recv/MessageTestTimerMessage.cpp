/******************************************************************************
*
* File name:   MessageTestTimerMessage.cpp
* Subsystem:   Platform Services
* Description: Test Message #1
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

#include "MessageTestTimerMessage.h"

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
MessageTestTimerMessage::MessageTestTimerMessage(const MailboxAddress& sourceAddress,
                                                 ACE_Time_Value timeout,
                                                 ACE_Time_Value restartInterval)
                                                :TimerMessage(sourceAddress, VERSION_NUMBER, timeout, restartInterval)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
MessageTestTimerMessage::MessageTestTimerMessage(const MailboxAddress& sourceAddress,
                                                 ACE_Time_Value timeout)
                                                :TimerMessage(sourceAddress, VERSION_NUMBER, timeout)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTestTimerMessage::~MessageTestTimerMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Id
// Design:
//-----------------------------------------------------------------------------
unsigned short MessageTestTimerMessage::getMessageId() const
{
   return MSGMGR_TEST_TIMER_MSG_ID;
}//end getMessageId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MessageTestTimerMessage::toString()
{
   string s = "MessageTestTimerMessage";
   return (s);
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

