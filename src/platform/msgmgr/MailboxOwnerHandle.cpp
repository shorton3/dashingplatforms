/******************************************************************************
*
* File name:   MailboxOwnerHandle.cpp
* Subsystem:   Platform Services
* Description: This class is a handle to a mailbox with owner priviledges. The
*              mailbox itself may be of local or distributed type.
*              The mailbox handles prevent application developers from getting
*              direct references to the mailboxes themselves.
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

#include <ace/Reactor.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxOwnerHandle.h"
#include "MailboxHandle.h"
#include "MailboxAddress.h"
#include "MailboxProcessor.h"

#include "platform/logger/Logger.h"

#include "platform/opm/OPM.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: Here, we increment the reference count and ensure that this
//              handle is meaningful. Applications will never have to call acquire()
//              themselves. This is done for them when they create their mailbox
//              or by the lookup service when they do a find.
// Design:     
//-----------------------------------------------------------------------------
MailboxOwnerHandle::MailboxOwnerHandle(MailboxBase* mailboxPtr)
     :mailboxPtr_(mailboxPtr)
{
   // Check the mailbox pointer.
   if (mailboxPtr_)
   {
      mailboxPtr_->acquire();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Attempt to create MailboxOwnerHandle with NULL/invalid mailbox pointer",0,0,0,0,0,0);
      // Abort the process (SHOULD NEVER GET HERE!)
      exit(ERROR);
   }//end else
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxOwnerHandle::~MailboxOwnerHandle()
{
   TRACELOG(DEVELOPERLOG, MSGMGRLOG, "MailboxOwnerHandle destructor/release called; application should consider deactivating first",0,0,0,0,0,0);
   mailboxPtr_->release();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Calls the post() method on the actual mailbox. Returns ERROR if
//              the handle has not been acquired else returns the error code
//              returned by the mailbox post() method.
// Design:
//-----------------------------------------------------------------------------
int MailboxOwnerHandle::post(MessageBase* messagePtr, const ACE_Time_Value* timeout)
{
   return (mailboxPtr_->post(messagePtr, timeout));
}//end post


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Schedule a Timer Message to expire at the specified timeout.
// Design:
//-----------------------------------------------------------------------------
long MailboxOwnerHandle::scheduleTimer(TimerMessage* timerMessagePtr)
{
   const void* argument = (const void*) timerMessagePtr;
   ACE_Reactor* reactor = mailboxPtr_->getReactor();
   if (reactor == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Select reactor is null. Possible attempt to schedule timer using Proxy Mailbox.",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else if (reactor->reactor_event_loop_done())
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Select reactor is not running",0,0,0,0,0,0);
      return ERROR;
   }//end else if
   long timerId = reactor->schedule_timer ( mailboxPtr_,                 /* Event Handler */
                         argument,                            /* Sent to handle_timeout() */
                         timerMessagePtr->getTimeout(),                        /* Timeout */
                         timerMessagePtr->getRestartInterval());    /* Restart after time */
   
   // Increment the active timer count
   if (timerId != ERROR)
      mailboxPtr_->incrementActiveTimers();
   else
      TRACELOG(ERRORLOG, MSGMGRLOG, "Error attempting to schedule timer with reactor",0,0,0,0,0,0);
   // Return the timer id
   return timerId;
}//end scheduleTimer


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Cancel an outstanding, active Timer Message
// Design:
//-----------------------------------------------------------------------------
int MailboxOwnerHandle::cancelTimer(long timerId, TimerMessage* timerMessagePtr)
{
   const void* tmpPtr = (const void*) timerMessagePtr;
   // By providing the pointer to the attached Timer Message, the Timer Message
   // itself will be deleted by the Reactor Framework.
   ACE_Reactor* reactor = mailboxPtr_->getReactor();
   if (reactor == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Select reactor is null. Possible attempt to cancel timer using Proxy Mailbox.",0,0,0,0,0,0);
      return ERROR;
   }//end if 
 
   // If the TimerMessage is OPM Poolable and Owned by the OPM, or if the
   // TimerMessage is reusable, then we don't want to delete it. Just cancel it
   // with the reactor.
   int result = 0;
   if (timerMessagePtr->isPoolable() && OPM::isCreatedByOPM((OPMBase*)timerMessagePtr))
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Deferring timer message delete to OPM release",0,0,0,0,0,0);
      result = reactor->cancel_timer (timerId);
      OPM_RELEASE((OPMBase*)timerMessagePtr);
   }//end if
   // Check to see if this timer is reusable. If so, it is the responsibility of the
   // application developer to delete it.
   else if (timerMessagePtr->isReusable())
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Deferring reusable timer message delete to the application",0,0,0,0,0,0);
      result = reactor->cancel_timer (timerId);
   }//end else if
   // Else if not OPM owned and not reusable, we can delete the Timer Message here
   else
   {
      result = reactor->cancel_timer (timerId, &tmpPtr);
   }//end else

   // Check the result
   if (result == 1)
   {
      mailboxPtr_->decrementActiveTimers();
      return OK;
   }//end if
   else
   {
      return ERROR;
   }//end else
}//end cancelTimer


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reset the timer interval for a specified outstanding, active Timer Message.
// Design:
//-----------------------------------------------------------------------------
int MailboxOwnerHandle::resetTimerInterval (long timerId, 
                                            TimerMessage* timerMessagePtr, 
                                            const ACE_Time_Value &newInterval)
{
   // If this timer's restart interval is set so that it will no longer restart,
   // then we need to make the Timer non-reusable.
   if (newInterval == ACE_Time_Value::zero)
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Resetting Timer restart interval to zero makes Timer Non-reusable",0,0,0,0,0,0); 
      timerMessagePtr->setNoLongerReusable();
   }//end if

   ACE_Reactor* reactor = mailboxPtr_->getReactor();
   if (reactor == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Select reactor is null. Possible attempt to reset timer using Proxy Mailbox.",0,0,0,0,0,0);
      return ERROR;
   }//end if
   return ( reactor->reset_timer_interval(timerId, newInterval) );
}//end resetTimerInterval


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Activates the mailbox
// Design:
//-----------------------------------------------------------------------------
int MailboxOwnerHandle::activate()
{
   return mailboxPtr_->activate(this);
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Deactivates the mailbox
// Design:
//-----------------------------------------------------------------------------
int MailboxOwnerHandle::deactivate()
{
   return mailboxPtr_->deactivate(this);
}//end deactivate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieves the next message from the mailbox queue. This is a 
//              blocking call.
// Design: Note that we could use the ACE_GUARD macros here instead...
//-----------------------------------------------------------------------------
MessageBase* MailboxOwnerHandle::getMessage(unsigned short timeout)
{
   MessageBase* message = NULL;

   getMessageMutex_.acquire();
   message = mailboxPtr_->getMessage(timeout);
   getMessageMutex_.release();
   return message;
}//end getMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieves the next message from the mailbox queue. This is a
//              blocking call.
// Design: Note that we could use the ACE_GUARD macros here instead...
//-----------------------------------------------------------------------------
MessageBase* MailboxOwnerHandle::getMessageNonBlocking()
{
   MessageBase* message = NULL;

   getMessageMutex_.acquire();
   message = mailboxPtr_->getMessageNonBlocking();
   getMessageMutex_.release();
   return message;
}//end getMessageNonBlocking


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve the mailbox debug flag
// Design:
//-----------------------------------------------------------------------------
int MailboxOwnerHandle::getDebugValue()
{
   return mailboxPtr_->getDebugValue();
}//end getDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the mailbox debug flag
// Design:
//-----------------------------------------------------------------------------
void MailboxOwnerHandle::setDebugValue(int debugValue)
{
   mailboxPtr_->setDebugValue(debugValue);
}//end setDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return whether or not this Mailbox is a remote proxy or not
// Design:
//-----------------------------------------------------------------------------
bool MailboxOwnerHandle::isProxy()
{
   return mailboxPtr_->isProxy();
}//end isProxy


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& MailboxOwnerHandle::getMailboxAddress()
{
   return mailboxPtr_->getMailboxAddress();
}//end getMailboxAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the sent message counter value
// Design:
//-----------------------------------------------------------------------------
unsigned int MailboxOwnerHandle::getSentCount()
{
   return mailboxPtr_->getSentCount();
}//end getSentCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Rename the mailbox address
// Design:
//-----------------------------------------------------------------------------
bool MailboxOwnerHandle::rename(const MailboxAddress& newRemoteAddress)
{
   return mailboxPtr_->rename(newRemoteAddress);
}//end rename


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return a regular Mailbox Handle to this same encapsulated Mailbox
// Design: This method call creates a Mailbox Handle on the heap. It then
//    becomes the callers responsibility to delete the regular handle when
//    finished with it to prevent a memory leak.
//-----------------------------------------------------------------------------
MailboxHandle* MailboxOwnerHandle::generateMailboxHandleCopy()
{
   MailboxHandle* mailboxHandle = new MailboxHandle(mailboxPtr_);
   return mailboxHandle;   
}//end generateMailboxHandleCopy


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MailboxOwnerHandle::toString()
{
   string s = "";
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

