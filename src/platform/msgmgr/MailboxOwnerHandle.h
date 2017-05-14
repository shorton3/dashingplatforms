/******************************************************************************
* 
* File name:   MailboxOwnerHandle.h 
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

#ifndef _PLAT_MAILBOX_OWNER_HANDLE_H_
#define _PLAT_MAILBOX_OWNER_HANDLE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Thread_Mutex.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxBase.h"
#include "TimerMessage.h"

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

// For C++ class declarations, we have one (and only one) of these access 
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors/destructors first)
// 5) instance data
//

/**
 * MailboxOwnerHandle is a handle to a mailbox with owner priviledges.
 * <p>
 * The mailbox handles prevent application developers from getting
 * direct references to the mailboxes themselves. This is the programmatic
 * interface that developers use to access the mailbox functionality.
 * <p>
 * A MailboxOwnerHandle may refer to a local mailbox or a remote type
 * mailbox (assuming that the application created the distributed mailbox), and
 * the Owner Handle enables the user/developer to perform activation/
 * deactivation on a mailbox.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class MailboxHandle;

class MailboxOwnerHandle
{
   public:

      /** Constructor. Stores a pointer to the mailbox */
      MailboxOwnerHandle(MailboxBase* mailboxPtr);

      /** Virtual Destructor */
      virtual ~MailboxOwnerHandle();

      /** 
       * Post a message to this mailbox 
       * @returns ERROR for an error; otherwise OK
       */
      virtual int post(MessageBase* messagePtr, const ACE_Time_Value* timeout = &ACE_Time_Value::zero);
                                                                                                           
      /** 
       * Schedule a Timer Message to expire at the specified timeout.
       * When the Timer expires, it is enqueued to the local mailbox.
       * @returns TimerId for this Timer, or ERROR
       */
      long scheduleTimer(TimerMessage* timerMessagePtr);

      /**
       * Cancel an outstanding, active Timer Message.
       * NOTE that the TimerMessage is provided here and is deleted when the timer
       * is canceled -if- the Timer is not Owned/Pooled by the OPM and its not reusable.
       * If the timer is Owned/Pooled by the OPM, then it will be released back into
       * the pool here. If the timer is reusable, then it is upto the application to
       * delete it.
       * @returns OK if cancellation succeeded and ERROR if the TimerId wasn't found
       */
      int cancelTimer(long timerId, TimerMessage* timerMessagePtr);

      /**
       * Reset the timer interval for a specified outstanding, active Timer Message. 
       * In other words, change how often the timer is periodically restarted. A
       * value of ACE_Time_Value::zero will cause the timer to NOT be restarted again.
       * @returns OK if successful, ERROR if not.
       */
      int resetTimerInterval (long timerId, TimerMessage* timerMessagePtr, const ACE_Time_Value &newInterval);

      /** 
       * This method activates the mailbox 
       * @returns OK on success; ERROR for failure
       */
      int activate();

      /** 
       * This method deactivates the mailbox
       * @returns OK on success; ERROR for failure
       */
      int deactivate();

      /**
       * Will block until a message is available.
       * May return NULL if no message available (if mailbox deactivated).
       */
      virtual MessageBase* getMessage(unsigned short timeout = 0);
                                                                                                           
      /**
       * Will not block until a message is available.
       * <p>
       * If the application has a need to do non-blocking calls to get
       * (or check to see if) messages pending on the mailbox queue, then
       * the application needs to implement this call -instead- of calling
       * MailboxProcess::processMailbox(). If this is indeed the case, then
       * the application will either need to implement the case switching
       * on the Message Type -or- implement the find for the MessageHandlerList
       * (functor lookup).
       * @return NULL if no message available (and if mailbox deactivated).
       */
      virtual MessageBase* getMessageNonBlocking();

      /** Returns the Debug flag value for this mailbox. */
      virtual int getDebugValue();

      /** Sets the Debug flag value for this mailbox. */
      virtual void setDebugValue(int debugValue);

      /** Return whether this mailbox is a remote proxy or not */
      virtual bool isProxy();

      /** Return the mailbox address */
      virtual MailboxAddress& getMailboxAddress();

      /** Return the sent message counter value */
      virtual unsigned int getSentCount();

      /** Rename the Mailbox address */
      bool rename(const MailboxAddress& newRemoteAddress);

      /**
       * Return a regular Mailbox Handle to this same encapsulated Mailbox.
       * NOTE: This method call creates a Mailbox Handle on the heap. It then
       * becomes the callers responsibility to delete the regular handle when 
       * finished with it to prevent a memory leak.
       */
      MailboxHandle* generateMailboxHandleCopy();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /** Pointer to the encapsulated mailbox */
      MailboxBase* mailboxPtr_;

   private:

      /** Default Constructor */
      MailboxOwnerHandle();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MailboxOwnerHandle(const MailboxOwnerHandle& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MailboxOwnerHandle& operator= (const MailboxOwnerHandle& rhs);

      /** Non-recursive Thread Mutex to protect against multiple threads calling getMessage */
      ACE_Thread_Mutex getMessageMutex_;

};

#endif
