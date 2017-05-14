/******************************************************************************
*
* File name:   MailboxBase.cpp
* Subsystem:   Platform Services
* Description: This Abstract class specifies an interface for all mailboxes.
*              It allows applications to activate and deactivate mailboxes,
*              post messages, and retrieve messages in a blocked or non-blocked
*              fashion.
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

#include <ace/Select_Reactor.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageBase.h"
#include "MailboxBase.h"
#include "MailboxAddress.h"
#include "MailboxLookupService.h"
#include "MailboxOwnerHandle.h"
#include "TimerMessage.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Atomic_Op_Ex<ACE_Thread_Mutex, int>;
template class ACE_Atomic_Op<ACE_Thread_Mutex, int>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Atomic_Op_Ex<ACE_Thread_Mutex, int>
#pragma instantiate ACE_Atomic_Op<ACE_Thread_Mutex, int>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

// Static shutdown flag for signaling child thread exit
ACE_Atomic_Op <ACE_Thread_Mutex, unsigned int> MailboxBase::isShuttingDown_ = FALSE;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxBase::MailboxBase()
   : debugValue_(0),           /* NOTE: Turn this back off (0) later...for debugging */
     isProxy_(false),
     selectReactor_(NULL),
     activeTimers_(0),
     ownerHandleWhoActivatedMe_(NULL),
     active_(FALSE),
     referenceCount_(0),
     receivedCount_(0),
     sentCount_(0)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxBase::~MailboxBase()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Indicate whether this mailbox is a Proxy side mailbox for
//              posting messages
// Design:
//-----------------------------------------------------------------------------
bool MailboxBase::isProxy()
{
   return isProxy_;
}//end isProxy


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: activates the mailbox. A mailbox is NOT active on creation.
// Design:      Making it virtual allows sub-classes to refine this behavior.
//-----------------------------------------------------------------------------
int MailboxBase::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   // Store the MailboxOwnerHandle who last called activate. We will use it
   // for final release (and destruction of this Mailbox)
   ownerHandleWhoActivatedMe_ = mailboxOwnerHandle;

   active_ = TRUE;

   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: deactivates the mailbox
// Design:      Making it virtual allows sub-classes to refine this behavior.
//-----------------------------------------------------------------------------
int MailboxBase::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   // Take care of compiler warning
   MailboxOwnerHandle* tempOwnerHandle __attribute__ ((unused)) = mailboxOwnerHandle;

   active_ = FALSE;

   return OK;
}//end deactivate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the sent message counter value
// Design:
//-----------------------------------------------------------------------------
unsigned int MailboxBase::getSentCount()
{
   return sentCount_.value();
}//end getSentCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the received message counter value
// Design:
//-----------------------------------------------------------------------------
unsigned int MailboxBase::getReceivedCount()
{
   return receivedCount_.value();
}//end getReceivedCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return number of active, outstanding Timers
// Design:
//-----------------------------------------------------------------------------
unsigned int MailboxBase::getActiveTimers()
{
   return activeTimers_.value();
}//end getActiveTimers


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Increment Active Timer count
// Design:
//-----------------------------------------------------------------------------
void MailboxBase::incrementActiveTimers()
{
   activeTimers_++;
}//end incrementActiveTimers


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Decrement Active Timer count
// Design:
//-----------------------------------------------------------------------------
void MailboxBase::decrementActiveTimers()
{
   activeTimers_--;
}//end decrementActiveTimers


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Acquire a handle to this mailbox. Here we do reference counting.
// Design: returns ERROR if failure; otherwise OK
//-----------------------------------------------------------------------------
int MailboxBase::acquire()
{
   referenceCount_++;
   return OK;
}//end acquire



//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Release the handle for this mailbox. Here we do reference counting.
// Design: When the referenceCount for this Mailbox returns to zero, the Mailbox
//   will de-register itself with the Mailbox Lookup Service and then delete itself.
//-----------------------------------------------------------------------------
void MailboxBase::release()
{
   referenceCount_--;
   if(referenceCount_ <= 0 )
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Release reference count has become zero, Deactivating, Deregistering, Garbage collecting mailbox",0,0,0,0,0,0);
      // If the mailbox is still activate, deactivate it
      if (isActive())
      {
         // Call child class deactivate (which will in turn call Deregister in the MLS)
         deactivate(ownerHandleWhoActivatedMe_);
      }//end if
      else
      {
         // Deregister with the Lookup Service
         MailboxLookupService::deregisterMailbox(ownerHandleWhoActivatedMe_);
      }//end else
      delete this;
      return;
   }//end if
}//end release


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description:
// Design:
//-----------------------------------------------------------------------------
bool MailboxBase::rename(const MailboxAddress& newRemoteAddress)
{
   // Do some dummy operation to prevent compiler warning for unused variable
   const MailboxAddress tmpAddr __attribute__ ((unused)) = newRemoteAddress;

   TRACELOG(WARNINGLOG, MSGMGRLOG, "Default base class rename() method called",0,0,0,0,0,0);
   return false;
}//end rename


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Called upon expiration of a Timer
// Design:
//-----------------------------------------------------------------------------
int MailboxBase::handle_timeout (const ACE_Time_Value &tv, const void* argument)
{
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Received timer expiration",0,0,0,0,0,0);

   // Decrement the active timer count
   decrementActiveTimers();

   // Cast the argument back to a base TimerMessage*
   TimerMessage* timerMessage = (TimerMessage*) argument;

   // Set the time that the Timer 'actually' expired inside the Timer Message
   // This can be inspected by the applications if need be
   timerMessage->setExpirationTime(tv);

   // Put the Timer Message in our Mailbox queue
   if (post(timerMessage) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Error enqueuing timer expiration message to mailbox",0,0,0,0,0,0);
   }//end if

   // Keep registered with the reactor
   return OK;
}//end handle_timeout


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return pointer to the Select Reactor
// Design:
//-----------------------------------------------------------------------------
ACE_Reactor* MailboxBase::getReactor()
{
   return selectReactor_;
}//end getReactor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MailboxBase::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Indicate whether this mailbox is active
// Design:
//-----------------------------------------------------------------------------
unsigned int MailboxBase::isActive()
{
   return active_.value();
}//end isActive


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Sets the activated state flag
// Design:
//-----------------------------------------------------------------------------
void MailboxBase::setActive(unsigned int active)
{
   active_ = active;
}//end setActive


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the sent message counter value
// Design:
//-----------------------------------------------------------------------------
void MailboxBase::incrementSentCount()
{
   sentCount_ ++;
}//end incrementSentCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the received message counter value
// Design:
//-----------------------------------------------------------------------------
void MailboxBase::incrementReceivedCount()
{
   receivedCount_ ++;
}//end incrementReceivedCount


//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

