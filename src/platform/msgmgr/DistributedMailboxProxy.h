/******************************************************************************
* 
* File name:   DistributedMailboxProxy.h 
* Subsystem:   Platform Services 
* Description: Proxy mailbox class for sending messages to the 'real'
*              DistributedMailbox on a remote node (or in another process).
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DISTRIBUTED_MAILBOX_PROXY_H_
#define _PLAT_DISTRIBUTED_MAILBOX_PROXY_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

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
 * DistributedMailboxProxy acts as a proxy mailbox sending messages to the 'real'
 * DistributedMailbox on a remote node (or in another process).
 * <p>
 * DistributedMailboxProxy performs serialization of the messages and interacts
 * with the transport layer to push (post) the message to the remote node or process.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class MailboxOwnerHandle;
class MessageBase;

class DistributedMailboxProxy : public MailboxBase
{
   public:

      /**
       * Post a message to the distributed remote mailbox.
       * Subclass implementations should examine the "active_" state.
       * Upon initial failure of the post, this proxy mailbox will close and re-open
       * the socket and attempt to post the message again automatically. If the
       * the repost fails, then ERROR will be returned, and it becomes the responsibility
       * of the application to retry the message after deleting the mailbox handle
       * and performing MailboxLookupService::find (which may return a redundant mate's
       * handle); or, the application can give up and delete the message off of the heap.
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
      static MailboxOwnerHandle* createMailbox(const MailboxAddress& remoteAddress);

      /** 
       * Activate the mailbox.
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

      /** Return the mailbox distributed address */
      MailboxAddress& getMailboxAddress();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:
                                                                                                                   
      /** Constructor */
      DistributedMailboxProxy(const MailboxAddress& remoteAddress);

      /** Virtual Destructor. Protected since this is a reference counted object. */
      virtual ~DistributedMailboxProxy();

   private:

      /** Default Constructor */
      DistributedMailboxProxy();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      DistributedMailboxProxy(const DistributedMailboxProxy& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DistributedMailboxProxy& operator= (const DistributedMailboxProxy& rhs);

      /** Required by base class MailboxBase. Not implemented */
      MessageBase* getMessage(unsigned short timeoutValue = 0); 

      /** Required by base class MailboxBase. Not implemented */
      MessageBase* getMessageNonBlocking();

      /** Address of the remote mailbox for distributed communications */
      MailboxAddress remoteAddress_;

      /** ACE Sock Connector from the Acceptor/Connector pattern. Socket client implementation */
      ACE_SOCK_Connector sockConnector_;

      /** ACE Sock Stream wrapper for the client socket. Used by the ACE Sock Connector */
      ACE_SOCK_Stream clientStream_;

      /** OPM Pool ID for storing MessageBuffer objects */
      int messageBufferPoolId_;

};

#endif
