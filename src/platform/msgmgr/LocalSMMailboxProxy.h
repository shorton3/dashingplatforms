/******************************************************************************
* 
* File name:   LocalSMMailboxProxy.h 
* Subsystem:   Platform Services 
* Description: Proxy mailbox class for sending messages to the 'real'
*              LocalSMMailbox in another process on the same node. The message
*              exchange is performed by enqueuing the message into an 
*              unbounded shared memory queue.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_LOCAL_SHARED_MEMORY_MAILBOX_PROXY_H_
#define _PLAT_LOCAL_SHARED_MEMORY_MAILBOX_PROXY_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalSMMailboxQueue.h"
#include "MailboxBase.h"
#include "MessageBuffer.h"

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
 * LocalSMMailboxProxy acts as a proxy mailbox sending messages to the 'real'
 * LocalSMMailbox in another process on the same node.
 * <p>
 * LocalSMMailboxProxy performs serialization of the messages and interacts
 * with the transport layer to push (post) the message into the shared memory
 * queue.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ACE_Process_Semaphore;
class MailboxOwnerHandle;
class MessageBase;

class LocalSMMailboxProxy : public MailboxBase
{
   public:

      /**
       * Post a message to the distributed remote mailbox.
       * Subclass implementations should examine the "active_" state.
       * @returns zero for an error, non-zero otherwise.
       */
      virtual int post(MessageBase* messagePtr, const ACE_Time_Value* timeout = &ACE_Time_Value::zero);

      /**
       * Allows applications to create a mailbox and get a handle to it.
       *
       * Since we do not want applications to have direct access to a
       * mailbox object this method is provided. Applications that create
       * mailboxes use this method to get a handle to their mailbox rather
       * than have direct access. The handle returned is a owner handle which
       * has all the privileges of the actual mailbox (getMessage/post etc).
       * This method creates the mailbox, activates it (which in turn
       * registers it with the lookup service) and then creates an owner
       * handle, acquires it and then returns that handle. Both the mailbox
       * and the returned handle are created on the heap.
       */
      static MailboxOwnerHandle* createMailbox(const MailboxAddress& localAddress);

      /**
       * Activate the mailbox. 
       * Here, activate always returns true since all we have to do is map into
       * shared memory. It returns true regardless if the Shared Memory Queue already
       * exists in shared memory or if we had to create it (meaning we are enqueuing,
       * but no one is listening).
       * <p>
       * For security, one must possess an Owner Handle to activate the mailbox
       * and register it with the Lookup Service.
       */
      virtual int activate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * Deactivate the mailbox.
       * For security, one must possess an Owner Handle to deactivate the mailbox
       * and deregister it with the Lookup Service.
       */
      virtual int deactivate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * Return the appropriate default timeout for the post()
       */
      virtual const ACE_Time_Value& getPostDefaultTimeout();

      /** Return the debug flag */
      virtual int getDebugValue();

      /** Set the debug flag */
      virtual void setDebugValue(int debugValue);

      /** Return the mailbox address */
      MailboxAddress& getMailboxAddress();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:
                                                                                                                   
      /** Constructor */
      LocalSMMailboxProxy(const MailboxAddress& localAddress, const char* queueName,
         const char* coordinatingMutexName);

      /** Virtual Destructor. Protected since this is a reference counted object. */
      virtual ~LocalSMMailboxProxy();

   private:

      /** Default Constructor */
      LocalSMMailboxProxy();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LocalSMMailboxProxy(const LocalSMMailboxProxy& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LocalSMMailboxProxy& operator= (const LocalSMMailboxProxy& rhs);

      /** Required by base class MailboxBase. Not implemented */
      MessageBase* getMessage(unsigned short timeoutValue = 0); 

      /** Required by base class MailboxBase. Not implemented */
      MessageBase* getMessageNonBlocking();

      /** Address of the remote mailbox for distributed communications */
      MailboxAddress localAddress_;

      /** Shared memory queue for exchanging MessageBase messages between processes */
      LocalSMMailboxQueue queue_;

      /** OPM Pool ID for storing MessageBuffer objects */
      int messageBufferPoolId_;

      /** OPM Pool ID for storing the LocalSMBuffer objects */
      int localSMBufferPoolId_;

      /** ACE Process Semaphore used to signal between enqueue/dequeue threads/processes
          when the queue is empty and non-empty */
      ACE_Process_Semaphore* processSemaphore_;
};

#endif
