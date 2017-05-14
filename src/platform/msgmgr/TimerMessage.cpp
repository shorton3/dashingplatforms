/******************************************************************************
*
* File name:   TimerMessage.cpp
* Subsystem:   Platform Services
* Description: Base class implementation for Timer Messages. All system
*              software Timer Messages (that can tolerate the MsgMgr concurrency
*              latency) must inherit from this class.
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

#include "TimerMessage.h"

#include "platform/common/Defines.h"
#include "platform/common/MessageIds.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
TimerMessage::TimerMessage(const MailboxAddress& sourceAddress,
                      unsigned int versionNumber,
                      ACE_Time_Value timeout, 
                      ACE_Time_Value restartInterval,
                      unsigned int sourceContextId,
                      unsigned int destinationContextId)
                     :ReusableMessageBase(sourceAddress, versionNumber, sourceContextId, destinationContextId),
                      timeout_(timeout),
                      restartInterval_(restartInterval)
{
   // Here, we will behave like either a MessageBase or ReusableMessageBase
   // depending on whether a non-default 'RestartInterval' parameter is given to
   // make the Timer periodically recurring.
   if (restartInterval > ACE_Time_Value::zero)
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "TimerMessage is reusable since non-default restart interval is given",0,0,0,0,0,0);
      isReusable_ = true;
   }//end if
   else
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "TimerMessage is Non-reusable since default restart interval is given",0,0,0,0,0,0);
      isReusable_ = false;
   }//end else
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
TimerMessage::~TimerMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the time interval after which the timer will expire
// Design:
//-----------------------------------------------------------------------------
ACE_Time_Value& TimerMessage::getTimeout()
{
   return timeout_;
}//end getTimeout


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the time interval after which the timer will be 
//              automatically rescheduled
// Design:
//-----------------------------------------------------------------------------
ACE_Time_Value& TimerMessage::getRestartInterval()
{
   return restartInterval_;
}//end getRestartInterval


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Sets the time that the Timer actually expires. This is called
//              by the MsgMgr framework. Applications should NOT call this method
// Design:
//-----------------------------------------------------------------------------
void TimerMessage::setExpirationTime(ACE_Time_Value expirationTime)
{
   expirationTime_ = expirationTime;
}//end setExpirationTime


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieves the 'actual' time that the Timer Expired. This value 
//              is set by the MsgMgr framework, and it is provided to the 
//              applications so that it can be used to 'judge' the amount of 
//              latency introduced by the Timer sitting inside the mailbox queue.
// Design:
//-----------------------------------------------------------------------------
ACE_Time_Value& TimerMessage::getExpirationTime()
{
   return expirationTime_;
}//end getExpirationTime


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the message Id
// Design:
//-----------------------------------------------------------------------------
unsigned short TimerMessage::getMessageId() const
{
   return MSGMGR_BASE_TIMER_ID;
}//end getMessageId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string TimerMessage::toString()
{
   string s = "MSGMGR_BASE_TIMER_ID";
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

