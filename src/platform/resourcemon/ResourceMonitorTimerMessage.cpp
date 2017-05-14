/******************************************************************************
*
* File name:   ResourceMonitorTimerMessage.cpp
* Subsystem:   Platform Services
* Description: Timer Message for periodically scheduling collection of resource
*              usage statistics.
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

#include "ResourceMonitorTimerMessage.h"

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
ResourceMonitorTimerMessage::ResourceMonitorTimerMessage(const MailboxAddress& sourceAddress,
                                                 ACE_Time_Value timeout,
                                                 ACE_Time_Value restartInterval)
                                                :TimerMessage(sourceAddress, VERSION_NUMBER, timeout, restartInterval)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ResourceMonitorTimerMessage::~ResourceMonitorTimerMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Id
// Design:
//-----------------------------------------------------------------------------
unsigned short ResourceMonitorTimerMessage::getMessageId() const
{
   return RESOURCE_MONITOR_TIMER_MSG_ID;
}//end getMessageId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string ResourceMonitorTimerMessage::toString()
{
   string s = "ResourceMonitorTimerMessage";
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

