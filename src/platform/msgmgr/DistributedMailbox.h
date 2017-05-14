/******************************************************************************
* 
* File name:   DistributedMailbox.h 
* Subsystem:   Platform Services 
* Description: Mailbox class for receiving messages from both Local and 
*              Distributed (out of this process, or on another node)
*              Applications. 
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DISTRIBUTED_MAILBOX_H_
#define _PLAT_DISTRIBUTED_MAILBOX_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Reactor.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/Thread_Mutex.h>

#include <map>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalMailbox.h"
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
 * DistributedMailbox is a mailbox class for receiving messages from both
 * Local and Distributed (out of this process, or on another node) Applications. 
 * <p>
 * Deserialization of the messages is performed here as well as the interaction 
 * with the transport layer. This class is used by the Distributed Mailbox Owner
 * for receiving messages that were posted and serialized by the Distributed
 * Mailbox proxy object on another node (or on the same node, but in a different
 * process).
 * <p>
 * The size of Message which may be exchanged is limited to MAX_MESSAGE_LENGTH
 * which is defined by the MessageBuffer class.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class MailboxOwnerHandle;

class DistributedMailbox : public LocalMailbox
{

   public:

      /**
       * Create a map for storing ACE_HANDLEs and their associated ACE_SOCK_Stream objects.
       * Each map entry corresponds to a single client 'connector' connection to this 
       * distributed mailbox.
       **/
      typedef map<ACE_HANDLE, ACE_SOCK_Stream*> ClientConnectorMap;

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
      static MailboxOwnerHandle* createMailbox(const MailboxAddress& remoteAddress);

      /** 
       * Method to allow application to rename a mailbox's remote address.
       * 
       * TBD: What are the redundancy implications and usage here??
       */ 
      virtual bool rename(const MailboxAddress& newRemoteAddress);

      /**
       * Activate the mailbox.
       * For security, one must possess an Owner Handle in order to activate
       * the mailbox and register it with the Lookup Service.
       */
      virtual int activate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * Deactivate the mailbox.
       * For security, one must possess an Owner Handle in order to deactivate
       * the mailbox and deregister it with the Lookup Service.
       */
      virtual int deactivate(MailboxOwnerHandle* mailboxOwnerHandle);

      /** Return the mailbox distributed address */
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
      DistributedMailbox(const MailboxAddress& distributedAddress);

      /** Virtual Destructor. Protected since this is a reference counted object. */
      virtual ~DistributedMailbox();

   private:

      /** Constructor */
      DistributedMailbox();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      DistributedMailbox(const DistributedMailbox& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DistributedMailbox& operator= (const DistributedMailbox& rhs);

      /**
       * This static method is called in a new thread context to start the ACE Reactor
       * event processing loop for distributed messages during mailbox activation. This will run the
       * event loop until the <ACE_Reactor::handle_events> or <ACE_Reactor::alertable_handle_events> methods
       * returns -1, the <end_reactor_event_loop> method is invoked, or the <ACE_Time_Value>
       * expires.
       * @param arg Pointer to the reactor that we are starting
       */
      static void startReactor(void* arg);

      /**
       * Overriden ACE_Event_Handler method. 
       * Called back automatically when a connection has been dropped
       * From ACE Docs:  Called when a <handle_*()> method returns -1 or when the
       * <remove_handler> method is called on an <ACE_Reactor>.  The <close_mask> 
       * indicates which event has triggered the <handle_close> method callback 
       * on a particular <handle>.
       **/
      int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask);

      /**
       * Overriden ACE_Event_Handler method.
       * Called back automatically when a connection has been established and upon receiving data
       **/
      int handle_input (ACE_HANDLE);

      /** Address of the remote mailbox for distributed communications */
      MailboxAddress distributedAddress_;

      /** ACE Sock Acceptor. Implementation of a server listener socket */
      ACE_SOCK_Acceptor* socketAcceptor_;

      /** Message Buffer object for deserialization of the MessageBase objects */
      MessageBuffer messageBuffer_;

      /** Map for associating each ACE_Handle (file descriptor) with its
          associated ACE_SOCK_Stream */
      ClientConnectorMap clientConnectorMap_;

      /** ACE Thread Mutex for protecting the client connector map */
      ACE_Thread_Mutex clientConnectorMapMutex_;

      /** ACE_Select_Reactor used for distributed message send/receive */
      ACE_Reactor* distributedReactor_;
};

#endif
