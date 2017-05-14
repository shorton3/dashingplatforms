/******************************************************************************
* 
* File name:   LocalSMMailbox.h 
* Subsystem:   Platform Services 
* Description: Mailbox class for receiving messages via Local Shared 
*              Memory Queue. This mailbox type allows processes to exchange
*              messages via shared memory, thus avoiding the overhead
*              caused by the network stack. Note that a LocalMailbox is 
*              transparently associated with this Mailbox type (which is necessary
*              to provide local-only access as well as concurrency for Timers). 
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_LOCAL_SM_MAILBOX_H_
#define _PLAT_LOCAL_SM_MAILBOX_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalMailbox.h"

#include "LocalSMMailboxQueue.h"

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
 * LocalSMMailbox is a mailbox class for receiving messages via Local Shared
 * Memory Queue. 
 * <p>
 * This mailbox type allows processes on the same node/machine
 * to exchange messages via shared memory, thus avoiding the
 * overhead caused by the network stack. Note that a LocalMailbox is
 * transparently associated with this Mailbox type (which is necessary
 * to provide local-only access as well as concurrency for Timers).
 * <p>
 * This class is used by the Local Shared Memory Mailbox Owner for receiving
 * messages that were posted and serialized by the Local Shared Memory 
 * Mailbox proxy object in another process.
 * <p>
 * The size of Message which may be exchanged is limited to MAX_MESSAGE_LENGTH
 * which is defined by the MessageBuffer class. To prevent unnecessary
 * churn, this Mailbox blocks on a process Mutex when the shared memory
 * queue is empty. This process mutex is released when the proxy class
 * inserts a message into the queue.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class ACE_Process_Semaphore;
class MailboxOwnerHandle;

class LocalSMMailbox : public LocalMailbox
{

   public:

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
       * For security, one must possess an Owner handle to activate the mailbox
       * and register it with the Lookup Service.
       */
      virtual int activate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * Deactivate the mailbox.
       * For security, one must possess an Owner handle to deactivate the mailbox
       * and deregister it with the Lookup Service.
       */
      virtual int deactivate(MailboxOwnerHandle* mailboxOwnerHandle);

      /** Return the mailbox address */
      MailboxAddress& getMailboxAddress();

      /**
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /**
       * Constructor - protected so that applications cannot create their own 
       * mailboxes and must use the static createMailbox() method
       */
      LocalSMMailbox(const MailboxAddress& localAddress, const char* queueName, 
         const char* coordinatingMutexName);

      /** Virtual Destructor. Protected since this is a reference counted object. */
      virtual ~LocalSMMailbox();

   private:

      /** Constructor */
      LocalSMMailbox();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LocalSMMailbox(const LocalSMMailbox& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LocalSMMailbox& operator= (const LocalSMMailbox& rhs);

      /** Start the shared memory processing thread */
      static void startSMProcessingThread();

      /** 
       * Handle receiving message from the shared memory queue
       */
      void handleSMMessages();

      /** Address of the local mailbox */
      MailboxAddress localAddress_;

      /** Shared memory queue for exchanging MessageBase messages between processes */
      LocalSMMailboxQueue queue_;

      /** ACE Process Semaphore used to signal between enqueue/dequeue threads/processes
          when the queue is empty and non-empty */
      ACE_Process_Semaphore* processSemaphore_;

      /** Static singleton instance */
      static LocalSMMailbox* localSMMailbox_;
};

#endif
