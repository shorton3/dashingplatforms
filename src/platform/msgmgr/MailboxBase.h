/******************************************************************************
* 
* File name:   MailboxBase.h 
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

#ifndef _PLAT_MAILBOX_BASE_H_
#define _PLAT_MAILBOX_BASE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>
#include <ace/Atomic_Op.h>
#include <ace/Event_Handler.h>
#include <ace/Thread_Mutex.h>
#include <ace/Time_Value.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxAddress.h"
#include "MessageBase.h"

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
 * MailboxBase is an abstract base class interface for all mailbox types. 
 * <p>
 * MailboxBase allows applications to activate and deactivate mailboxes, 
 * post messages, and retrieve messages in a blocked or non-blocked fashion.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class MailboxOwnerHandle;

class MailboxBase : public ACE_Event_Handler
{
   public:

      /**
       * This method activates the mailbox. A mailbox is NOT active on creation.
       * Making it virtual allows sub-classes to refine this behavior. For
       * security, one must possess an Owner Handle to the mailbox in order to
       * activate it. 
       * @returns ERROR on failure, OK on success
       */
      virtual int activate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * This method deactivates the mailbox.
       * Making it virtual allows sub-classes to refine this behavior. For
       * security, one must possess an Owner Handle to the mailbox in order to
       * deactivate it.
       * @returns ERROR on failure, OK on success
       */
      virtual int deactivate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * Subclass implementations should return the appropriate default timeout
       * for the post()
       */
      virtual const ACE_Time_Value& getPostDefaultTimeout() = 0;

      /**
       * Post a message to this mailbox.
       * Subclass implementations should examine the "active_" state.
       * @returns ERROR for an error, OK otherwise.
       */
      virtual int post(MessageBase* messagePtr, const ACE_Time_Value* timeout = &ACE_Time_Value::zero) = 0;

      /**
       * Will block until an message is available.
       * May return NULL if no message available (if mailbox deactivated).
       */
      virtual MessageBase* getMessage(unsigned short timeoutValue = 0) = 0;

      /**
       * Will not block until an message is available.
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
      virtual MessageBase* getMessageNonBlocking() = 0;

      /** 
       * Acquire a handle to this mailbox. Here we do reference counting.
       * We implement a reference count mechanism to keep track of mailbox objects. 
       * When an application wants to access a mailbox it acquires a handle 
       * to the mailbox which increments the reference count for the mailbox. 
       * When the application is done with a mailbox (for a transient usage) it
       * releases the mailbox which decrements the reference count. When the 
       * reference count returns to 0 the mailbox deletes itself.
       * @returns ERROR if failure; otherwise OK
       */
      virtual int acquire();

      /** 
       * Release the handle for this mailbox. Here we do reference counting.
       * When the referenceCount for this Mailbox returns to zero, the Mailbox
       * will de-register itself with the Mailbox Lookup Service and then
       * delete itself.
       */
      virtual void release();

      /** Return the debug flag */
      virtual int getDebugValue() = 0;

      /** Set the debug flag */
      virtual void setDebugValue(int debugValue) = 0;

      /** Return the Mailbox address */
      virtual MailboxAddress& getMailboxAddress() = 0;

      /** Return the sent message counter value */
      unsigned int getSentCount();

      /** Return the received message counter value */
      unsigned int getReceivedCount();

      /** Rename the Mailbox address */
      virtual bool rename(const MailboxAddress& newRemoteAddress);

      /**
       * Overriden ACE_Event_Handler method.
       * <p>
       * Method to handle timer expirations from the ACE reactor. This is 
       * base class functionality since all Mailboxes should have Timer capability.
       **/
      int handle_timeout (const ACE_Time_Value &tv, const void* argument);

      /** Return number of active, outstanding Timers */
      unsigned int getActiveTimers();

      /** Increment Active Timer count */
      void incrementActiveTimers();

      /** Decrement Active Timer count */
      void decrementActiveTimers();

      /** Return pointer to the select reactor used for event dispatch */
      ACE_Reactor* getReactor();

      /** Indicate whether this mailbox is a Proxy side mailbox for posting messages */
      bool isProxy();

      /**
       * Indicates whether the Mailbox has been activated or not
       * @returns TRUE or FALSE
       */
      unsigned int isActive();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /** Constructor */
      MailboxBase();

      /** Virtual Destructor. Protected since this is a reference counted object. */
      virtual ~MailboxBase();

      /** Sets the activate state flag */
      void setActive(unsigned int active);

      /** Increments the Mailbox's received message count */
      void incrementReceivedCount();

      /** Increments the Mailbox's sent message count */
      void incrementSentCount();

      /** Debug Flag */
      int debugValue_;

      /** Flag to indicate whether this mailbox is a Proxy side mailbox for posting messages */
      bool isProxy_;

      /** ACE_Select_Reactor used for event dispatch. This only gets instantiated by non-proxy mailboxes */
      ACE_Reactor* selectReactor_;

      /** Flag to indicate that the Mailbox is being shutdown. Set in the Mailbox's destructor for loops to see */
      static ACE_Atomic_Op <ACE_Thread_Mutex, unsigned int> isShuttingDown_;

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MailboxBase(const MailboxBase& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MailboxBase& operator= (const MailboxBase& rhs);

      /** Number of Active outstanding Timers */
      ACE_Atomic_Op <ACE_Thread_Mutex, unsigned int> activeTimers_;

      /** Pointer to the MailboxOwnerHandle that last activated the Mailbox. This
          will be used when we do final release (and destruction) of this Mailbox */
      MailboxOwnerHandle* ownerHandleWhoActivatedMe_;

      /** Determines whether or not new messages can be posted and retrieved. */
      ACE_Atomic_Op <ACE_Thread_Mutex, unsigned int> active_;

      /**
       * Indicates the number of handles to this mailbox
       */
      ACE_Atomic_Op <ACE_Thread_Mutex, unsigned int> referenceCount_;

      /** Counter for the number of Received Messages */
      ACE_Atomic_Op <ACE_Thread_Mutex, unsigned int> receivedCount_;

      /** Counter for the number of Sent Messages */
      ACE_Atomic_Op <ACE_Thread_Mutex, unsigned int> sentCount_;
};

#endif
