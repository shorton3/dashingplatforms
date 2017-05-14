/******************************************************************************
* 
* File name:   GroupMailboxProxy.h 
* Subsystem:   Platform Services 
* Description: Proxy mailbox class for sending messages to the 'real'
*              GroupMailbox(es) on a remote node (or in another process). Since
*              this is multicast communication, multiple mailboxes may 
*              simultaneously receive each message.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_GROUP_MAILBOX_PROXY_H_
#define _PLAT_GROUP_MAILBOX_PROXY_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/SOCK_Dgram_Bcast.h>
#include <ace/SOCK_Dgram_Mcast.h>

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
 * GroupMailboxProxy acts as a proxy mailbox sending messages to the 'real'
 * GroupMailbox(es) on a remote node (or in another process).
 * <p>
 * GroupMailboxProxy performs serialization of the messages and interacts
 * with the transport layer to push (post) the message to the remote node or
 * process via the ACE Multicast Datagram protocol or the ACE Broadcast Datagram
 * protocol depending on the IP Address range of the given target. Since this is
 * multicast communication, multiple mailboxes may receive a single message
 * posted from here.
 * <p>
 * The size of Message which may be exchanged is limited to MAX_MESSAGE_LENGTH
 * which is defined by the MessageBuffer class.
 * <p>
 * Its not possible to use OPM to store the required Message_Block objects for
 * passing into the ACE mechanism (formerly RMCast). Because we have limited control of
 * the reliable resend mechanism, we cannot tell when the message blocks can
 * safely be cleaned and returned to the pool. Thus we must rely on the ACE
 * reference counting mechanism (contained when ACE_Message_Block is created
 * on the heap).
 * <p>
 * Since we will probably have other processes on this same node who are interested
 * in receiving/joining for this multicast group, we need to turn on multicast
 * loopback. If we do not, then the network stack will not send the message back
 * to its listeners -- it will only send it externally. Actually, in Linux, this
 * should be enabled by default!
 * <p> 
 * In other words, when multicast loopback is disabled, this means two applications
 * on the same machine who join the same multicast group will get each other's packets.
 * This mode has the best performance if there is always only one application joining
 * the multicast group on the machine.
 * <p> 
 * When loopback is enabled, it enables multiple applications on one machine to join
 * the same multicast group. The packets are looped back from the stack, and each
 * application is responsible for filtering out unwanted packets.
 * <p>
 * If we experience problems with Multicast packets getting from one network to
 * another, we need to probably change the multicast TTL. By default, it is set
 * to 1 to allow for only 1 hop.
 * <p>
 * OLD NOTE (kept for posterity): At the time of this coding, a source code change had to be made to 
 * the ACE RMCast RMCast_IO_UDP.i implementation file in the library (and the
 * library recompiled). This is a known bug that should be fixed in the next
 * major release of ACE (beyond 5.4).
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class MailboxOwnerHandle;
class MessageBase;

class GroupMailboxProxy : public MailboxBase
{
   public:

      /**
       * Post a message to the group mailbox.
       * Subclass implementations should examine the "active_" state.
       * Upon initial failure of the post, this proxy mailbox will close and re-open
       * the socket and attempt to post the message again automatically. If the
       * the repost fails, then ERROR will be returned, and it becomes the responsibility
       * of the application to retry the message after deleting the mailbox handle
       * and performing MailboxLookupService::find (which may return a redundant mate's
       * handle); or, the application can give up and delete the message off of the heap.
       * @returns ERROR upon failure; OK otherwise.
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
       * @param groupAddress source mailbox address that this mailbox will have
       * @param multicastLoopbackEnabled set to TRUE to enable multicast loopback
       * @param multicastTTL If we experience problems with Multicast packets getting
       *    from one network to another, we need to probably change the TTL. By default,
       *    it is set to 1 to allow for only 1 hop.  (within the same subnet, routers
       *    will not forward). Also, setting to 0 will restrict to within the same
       *    host.
       * @returns pointer to a mailbox owner handle
       */
      static MailboxOwnerHandle* createMailbox(const MailboxAddress& groupAddress,
         unsigned int multicastLoopbackEnabled = TRUE, unsigned int multicastTTL = 1);

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

      /** Return the mailbox group address */
      MailboxAddress& getMailboxAddress();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /** Constructor */
      GroupMailboxProxy(const MailboxAddress& groupAddress, unsigned int multicastLoopbackEnabled_,
          unsigned int multicastTTL_);

      /** Virtual Destructor. Protected since this is a reference counted object. */
      virtual ~GroupMailboxProxy();

   private:

      /** Default Constructor */
      GroupMailboxProxy();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      GroupMailboxProxy(const GroupMailboxProxy& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      GroupMailboxProxy& operator= (const GroupMailboxProxy& rhs);

      /** Required by base class MailboxBase. Not implemented */
      MessageBase* getMessage(unsigned short timeoutValue = 0); 

      /** Required by base class MailboxBase. Not implemented */
      MessageBase* getMessageNonBlocking();

      /** Address of the remote group mailbox for communications */
      MailboxAddress groupAddress_;

      /** OPM Pool ID for storing MessageBuffer objects */
      int messageBufferPoolId_;

      /** ACE Multicast Datagram socket */
      ACE_SOCK_Dgram_Mcast* multicastSocket_;

      /** ACE Broadcast Datagram socket */
      ACE_SOCK_Dgram_Bcast* broadcastSocket_;

      /** set to TRUE if multicast loopback is enabled for this mailbox */
      unsigned int multicastLoopbackEnabled_;

      /** TTL value which controls the number of hops allowed for each multicast packet.
          Valid values from 0 to 255 are allowed */
      unsigned int multicastTTL_;

      /** Set to true if the socket we are using is multicast; otherwise, false for
          broadcast */
      bool isMulticast_;
};

#endif
