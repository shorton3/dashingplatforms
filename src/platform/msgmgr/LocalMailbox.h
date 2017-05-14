/******************************************************************************
* 
* File name:   LocalMailbox.h 
* Subsystem:   Platform Services 
* Description: LocalMailbox is a subclass of MailboxBase.  It is used solely for
*              local messages and does not support receipt of messages from 
*              remote nodes in the system. This class may be subclassed in
*              order to derive more complex Mailbox implementations.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_LOCAL_MAILBOX_H_
#define _PLAT_LOCAL_MAILBOX_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Condition_Thread_Mutex.h>
#include <ace/Manual_Event.h>
#include <ace/Message_Queue.h>
#include <ace/Reactor.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxBase.h"

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
 * LocalMailbox is a subclass of MailboxBase that provides support for receiving
 * local messages only (within the same process space). 
 * <p>
 * LocalMailbox does not support receipt of messages from remote nodes in the
 * system. This class implements only a portion of the MailboxBase functionality
 * and leaves the remaining functionality virtual. This class may be subclassed
 * in order to derive move complex Mailbox implementations.
 * <p>
 * Note that the LocalMailbox exchanges messages by queuing and dequeuing
 * pointers to the messages. For optimal performance, no serialization/
 * deserialization of messages is performed here. Also, there is no limit on the
 * size of the MessageBase messages exchanged by the Local Mailbox (this is not
 * true of the other Mailbox types, which are bound by MAX_MESSAGE_LENGTH).
 * <p>
 * LocalMailbox handles the prioritization of MessageBase messages. Using the 
 * ACE Message Queue, the FIFO order of messages is maintained when messages of
 * the same priority are inserted consecutively.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class MailboxOwnerHandle;

class LocalMailbox : public MailboxBase
{
   public:

      /** 
       * Activate the mailbox.
       * For security, one must possess an Owner Handle to the mailbox in order
       * to activate it and register it with the Lookup Service.
       */
      virtual int activate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * Deactivate the mailbox.
       * For security, one must possess an Owner Handle to the mailbox in order
       * to deactivate it and deregister it with the Lookup Service.
       */
      virtual int deactivate(MailboxOwnerHandle* mailboxOwnerHandle);

      /** Return the post timeout */
      virtual const ACE_Time_Value& getPostDefaultTimeout();

      /**
       * Post a message to this mailbox.
       * @returns ERROR for an error, OK otherwise.
       */
      virtual int post(MessageBase* messagePtr, const ACE_Time_Value* timeout = &ACE_Time_Value::zero);

      /**
       * Will block until a message is available. Note that all non-Proxy
       * Mailbox types use the LocalMailbox's getMessage method to dequeue
       * MsgMgr messages.
       * May return NULL if no message available (if mailbox deactivated).
       */
      virtual MessageBase* getMessage(unsigned short timeoutValue = 0);

      /**
       * Will not block until a message is available.
       * <p>
       * If the application has a need to do non-blocking calls to get
       * (or check to see if) messages pending on the mailbox queue, then
       * the application needs to implement this call -instead- of calling
       * MailboxProcessor::processMailbox(). If this is indeed the case, then
       * the application will either need to implement the case switching
       * on the Message Type -or- implement the find for the MessageHandlerList
       * (functor lookup).
       * <p>
       * Note that all non-Proxy Mailbox types use the LocalMailbox's getMessage
       * method to dequeue MsgMgr messages.
       * @return NULL if no message available (and if mailbox deactivated).
       */
      virtual MessageBase* getMessageNonBlocking();

      /** Returns the Debug flag value for this mailbox. */
      virtual int getDebugValue();

      /** Sets the Debug flag value for this mailbox. */
      virtual void setDebugValue(int debugValue);

      /** Return the mailbox address */
      MailboxAddress& getMailboxAddress();

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

   protected:

      /**
       * Default Constructor - protected so that applications cannot
       * create their own mailboxes and must use the static createMailbox() method
       */
      LocalMailbox(const MailboxAddress& localAddress);

      /** Virtual Destructor. Protected since this is a reference counted object */
      virtual ~LocalMailbox();

      /**
       * This static method is called in a new thread context to start the ACE Reactor
       * event processing loop during mailbox activation. This will run the event loop until the
       * <ACE_Reactor::handle_events> or <ACE_Reactor::alertable_handle_events> methods
       * returns -1, the <end_reactor_event_loop> method is invoked, or the <ACE_Time_Value>
       * expires.
       * @param arg Pointer to the reactor that we are going to start
       */
      static void startReactor(void* arg);

      /** Address of this Mailbox */
      MailboxAddress localAddress_;

      /** Message queue for storing received messages */
      ACE_Message_Queue<ACE_MT_SYNCH> messageQueue_;

   private:

      /** Default Constructor */
      LocalMailbox();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LocalMailbox(const LocalMailbox& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LocalMailbox& operator= (const LocalMailbox& rhs);

      /** OPM Pool ID for storing Message Block Wrapper objects */
      int messageBlockWrapperPoolID_;

      /**
       * ACE Manual Event for controlling enqueue/dequeue thread behavior; when the queue is
       * empty the dequeue thread will block on this event until the enqueue thread signals
       * it that the queue is no longer empty.
       * Here we can use the Manual Event instead of a Process Semaphore since both sides
       * (enqueue/dequeue) are guaranteed to be in the same process.
       */
      ACE_Manual_Event manualEvent_;
};

#endif
