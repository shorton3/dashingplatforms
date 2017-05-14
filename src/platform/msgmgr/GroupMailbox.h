/******************************************************************************
* 
* File name:   GroupMailbox.h 
* Subsystem:   Platform Services 
* Description: Mailbox class for receiving messages from both Local and 
*              Group Mailbox-based Applications (based on ACE Multicast
*              Datagram protocol or ACE Broadcast Datagram protocol). 
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_GROUP_MAILBOX_H_
#define _PLAT_GROUP_MAILBOX_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Reactor.h>
#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/SOCK_Dgram_Bcast.h>

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
 * GroupMailbox is a mailbox class for receiving messages from both
 * Local and Group Mailbox-based Applications (based on ACE Multicast
 * Datagram protocol or ACE Broadcast Datagram protocol). If the IP address
 * provided to the GroupMailbox is in the reserved Multicast address range,
 * then multicast socket communication will be used; otherwise Broadcast 
 * socket communication will be used.
 * <p>
 * Deserialization of the messages is performed here as well as the interaction 
 * with the transport layer. This class is used by the Group Mailbox Owner
 * for receiving messages that were posted and serialized by the Group 
 * Mailbox proxy object on another node (or on the same node, but in a different
 * process). Since this is multicast communication, multiple mailboxes may
 * receive a single message posted by the Group Mailbox proxy object.
 * <p>
 * The size of Message which may be exchanged is limited to MAX_MESSAGE_LENGTH
 * which is defined by the MessageBuffer class.
 * <p>
 * Several tasks need to be performed to prepare Linux to participate in
 * Multicast communications. First a default route for all Multicast traffic
 * must be provided (for each interface that will participate):
 *   /sbin/route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
 * This route can be added permanently by either 
 *   a.) adding a file /etc/sysconfig/networking/devices/eth0.route with contents:
 *          ADDRESS0=192.168.30.0 NETMASK0=255.255.255.0 GATEWAY0=192.168.2.1
 *          ADDRESS1=192.168.31.0 NETMASK1=255.255.255.0 GATEWAY1=192.168.3.1
 *   b.) adding a file /etc/sysconfig/network-scripts/route-eth0 with contents:
 *          192.168.0.0/24 via 152.3.182.1
 * Note one file per interface. (See the Linux documentation from Redhat)
 * <p>
 * Remember that in IP Multicast, 224.0.0.1 is the default "all-hosts" group,
 * 224.0.0.2 is the default "all-routers" group for multicast routers,
 * 224.0.0.4 is the "all DVMRP routers" group, 224.0.0.5 "all OSPF routers",
 * and 224.0.0.13 is the "all PIM routers" group. Although it is not needed for
 * this portion of the platform, for Linux to be setup as a multicast router, the 
 * following additional step is necessary to tell the kernel to start forwarding
 * the multicast packets (turns on IP Forwarding):
 *   echo 1 > /proc/sys/net/ipv4/ip_forward
 * Note that I believe it is possible to accomplish without modifying proc; rather
 * I believe we can edit /etc/sysctl.conf.
 * <p>
 * By performing this ping command, all local multicast capable hosts will respond
 * since they join the 224.0.0.1 group by default upon startup:
 *   ping -t 1 -c 2 224.0.0.1 
 * Additional debugging information can be found in /proc/net/igmp and /proc/net/dev_mcast.
 * <p>
 * Note that the number of Multicast group subscriptions are limited by the kernel.
 * The default is currently 20, but you can tune it either by recompiling the
 * kernel, or at run-time, using sysctl.  For example:
 *   % sysctl net.ipv4.igmp_max_memberships
 *   net.ipv4.igmp_max_memberships = 20
 *   % sysctl -w net.ipv4.igmp_max_memberships=50
 *   net.ipv4.igmp_max_memberships = 50 
 * <p>
 * Just for reference, here is an example of default static IP contents of the
 * /etc/sysconfig/network-scripts/ifcfg-<dev> file:
 *   USERCTL=no
 *   PEERDNS=yes
 *   TYPE=Ethernet
 *   DEVICE=eth1
 *   HWADDR=00:03:6d:1e:58:18
 *   BOOTPROTO=none
 *   ONBOOT=yes
 *   IPADDR=10.10.0.1
 *   NETMASK=255.255.255.0
 *   NETWORK=10.10.0.0
 *   BROADCAST=10.10.0.255
 *   IPV6INIT=no
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class MailboxOwnerHandle;

class GroupMailbox : public LocalMailbox
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
       * @param groupAddress source mailbox address that this mailbox will have
       * @param multicastLoopbackEnabled set to TRUE to enable multicast loopback
       * @param multicastTTL If we experience problems with Multicast packets getting
       *    from one network to another, we need to probably change the TTL. By default,
       *    it is set to 1 to allow for only 1 hop (within the same subnet, routers
       *    will not forward). Also, setting to 0 will restrict to within the same
       *    host.
       * @returns pointer to a mailbox owner handle
       */
      static MailboxOwnerHandle* createMailbox(const MailboxAddress& groupAddress,
         unsigned int multicastLoopbackEnabled = TRUE, unsigned int multicastTTL = 1);

      /** 
       * Method to allow application to rename a mailbox's group address.
       * 
       * TBD: What are the redundancy implications and usage here??
       */ 
      virtual bool rename(const MailboxAddress& newGroupAddress);

      /**
       * Activate the mailbox.
       * For security, one must possess an Owner Handle in order to activate the
       * mailbox and register it with the Lookup Service.
       */
      virtual int activate(MailboxOwnerHandle* mailboxOwnerHandle);

      /**
       * Deactivate the mailbox.
       * For security, one must possess an Owner Handle in order to deactivate the
       * mailbox and deregister it with the Lookup Service.
       */
      virtual int deactivate(MailboxOwnerHandle* mailboxOwnerHandle);

      /** Return the mailbox group address */
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
      GroupMailbox(const MailboxAddress& groupAddress, unsigned int multicastLoopbackEnabled_,
          unsigned int multicastTTL_);

      /** Virtual Destructor. Protected since this is a reference counted object. */
      virtual ~GroupMailbox();

   private:

      /**
       * This static method is called in a new thread context to start the ACE Reactor
       * event processing loop for group messages during mailbox activation. This will run the
       * event loop until the <ACE_Reactor::handle_events> or <ACE_Reactor::alertable_handle_events> methods
       * returns -1, the <end_reactor_event_loop> method is invoked, or the <ACE_Time_Value>
       * expires.
       * @param arg Pointer to the reactor that we are going to start
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

      /** Constructor */
      GroupMailbox();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      GroupMailbox(const GroupMailbox& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      GroupMailbox& operator= (const GroupMailbox& rhs);

      /** Message Buffer object for deserialization of the MessageBase objects */
      MessageBuffer messageBuffer_;

      /** Address of the group mailbox for multicast communications */
      MailboxAddress groupAddress_;

      /** ACE Multicast Datagram socket */
      ACE_SOCK_Dgram_Mcast* multicastSocket_;

      /** ACE Broadcast Datagram socket */
      ACE_SOCK_Dgram_Bcast* broadcastSocket_;

      /** ACE_Select_Reactor used for group message signaling and send/receive */
      ACE_Reactor* groupReactor_;

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
