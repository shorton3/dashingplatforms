/******************************************************************************
* 
* File name:   MailboxLookupService.h 
* Subsystem:   Platform Services 
* Description: This class represents the mailbox registry and provides methods
*              for applications to register, find and deregister their mailboxes
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MAILBOX_LOOKUP_SERVICE_H_
#define _PLAT_MAILBOX_LOOKUP_SERVICE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <map>
#include <string>
#include <vector>

#include <ace/Thread_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxAddress.h"

#include "platform/common/Defines.h"

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
 * MailboxLookupService represents the mailbox registry.
 * <p>
 * MailboxLookupService provides an interface for applications to register,
 * find and deregister their mailboxes. This interface is the main mechanism
 * for identifying redundancy/high availability information (ie. which node
 * is active) since applications have the flexibility of registering
 * their mailbox in a concrete fashion (for example, bound to a particular
 * shelf number and slot number) or in a logical fashion (for example, 2 cards
 * performing the same logical role, where one is the active redundant mate,
 * and the other is the standby).
 * <p>
 * Flow for Proxy and Non-Proxy Mailboxes is as follows:
 * - Applications get access to mailboxes by either performing a find on a
 *   Mailbox through the LookupService or by creating their own by invoking
 *   one of the 'createMailbox' methods. Both methods return a mailbox handle
 *   allocated on the heap (handle is reference counted). When a requested 'find'
 *   address is of LocationType LOCAL_MAILBOX, then the returned handle will point
 *   to a Local Mailbox (and posted messages will be handled by the Local Mailbox
 *   post routine). If the requested Mailbox address has a remote LocationType
 *   (Distributed, LocalSM, or Group), then the returned handle will point to
 *   a Proxy Mailbox (and posted messages will be serialized by the Proxy
 *   Mailbox's post routine for remote delivery).
 * - Applications can use the handle to control Mailboxes that they own, or to
 *   post messages to other Mailboxes, but applications are NOT allowed to create
 *   copies of the handle or make their own handle to a mailbox.
 * - If a handle is explicitly deleted (on the heap), causing its destructor to be
 *   called, its internal reference count gets decremented.
 * - If a post operation using a mailbox handle fails, then the handle should be
 *   destroyed (should be explicitly deleted off the heap). Note that if a post
 *   operation returns a failure, the MsgMgr framework has ALREADY attempted to
 *   re-establish communication to the remote mailbox and retried the message again.
 * - After the handle is deleted, the application can later 're-invoke'
 *   MailboxLookupService.find(); a reconnect will be attempted, and a newly
 *   allocated handle will be returned.
 * - If upon handle destructor/delete, if the reference count goes to zero, then
 *   the MailboxProxy/Mailbox will be removed from the LookupService AND deleted.
 *   In this way, if an application needs a mailbox connection for initial
 *   configuration but not afterwards, then the resources for that mailbox
 *   connection can be cleaned up/released during runtime. Only needed connections
 *   can/will be preserved in the LookupService.
 * - This means that applications can either call find everytime they need a
 *   mailbox reference (with the risk that the connection will be re-established
 *   each time it is needed--if no one else has a reference to that mailbox); or
 *   the applications can store the mailbox handle reference and re-use it, but
 *   they must do some simple error handling in the event that 'post' fails (by
 *   deleting the reference and again doing a 'find'). An advantage to this
 *   strategy is that re-invoking 'find' allows the MailboxLookupService to
 *   substitute a redundant mailbox reference in the event of a failure.
 * <p>
 * Some guidelines rules for how the MsgMgr works are as follows:
 * - Only way for a Mailbox to be deleted is for its reference count to go to zero
 *   (by releasing all handles). This always causes the Mailbox to be deactivated if
 *   still active and performs Lookup Service deregistration as well)
 * - Activate/Deactivate can only be called with an Owner Handle to a Mailbox.
 *   For Proxy Mailboxes, this can only be done by the MailboxLookupService.
 *   For Non-Proxy Mailboxes, the owning application can do this (but the
 *   LookupService can deactivate a mailbox too if it is replacing an entry in
 *   its registry with a duplicate). Note that all active Mailboxes must be
 *   registered with the MailboxLookupService; and ONLY active Mailboxes will
 *   remain registed with the MailboxLookupService.
 * - If an application deletes its MailboxOwnerHandle, then this performs release().
 *   However, if the application is going to delete its Owner Handle, it should
 *   consider deactivating the Mailbox first.
 * - If an application deletes a MailboxHandle, then this performs release().
 *   Deleting an Owner Handle or regular Handle is the only way that release,
 *   reference count decrement, is called).
 * - If a Mailbox's activate() fails, then the Mailbox does NOT get registered
 *   with the LookupService (full rollback to a deactivated state is performed).
 * - If a post operation fails, the Distributed and Group Proxy Mailboxes will
 *   automatically try to re-establish the connection and re-post the message.
 *   If the re-post fails, then the application is responsible for explicitly
 *   deleting the handle and performing re-find. This is necessary to allow
 *   the MailboxLookupService to substitute a redundant mate if available.
 *   Additionally, if post ultimately fails and the posting is aborted (application
 *   gives-up), then the application is responsible for deleting the Message which
 *   was allocated on the heap. For the normal behavior, if the post is successful,
 *   the MsgMgr framework will delete the message.
 * - MailboxLookupService::register/deregister are only involved in add/delete
 *   mailboxes to its containers; the do not attempt to perform deactivate or release
 *   on mailbox handles.
 * - MailboxLookupService::find can return a new handle to an already existing/registered
 *   Mailbox (remember, handle is allocated on heap)
 * - MailboxLookupService maintains three separate registries that get populated
 *   when applications call registerMailbox/deregisterMailbox:
 *      - one for LocalMailboxes for which the MailboxAddress and a reference/
 *        handle to the actual mailbox is stored. These are for local
 *        applications to perform a 'find' on and post messages to.
 *      - one for remote type Non-Proxy Mailboxes for which ONLY the Mailbox
 *        Address is stored. These addresses are tracked across cards via
 *        the DiscoveryManager. Since remote type non-proxy mailboxes
 *        also have an underlying Local Mailbox implementation as well,
 *        their local Mailbox equivalent gets stored in the LocalMailbox
 *        registry (for the case where a Local, same process application
 *        wishes to post a message to a Distributed Mailbox's Local Mailbox
 *        queue--without going through message serialization/deserialization).
 *        NOTE: For this reason, it is NOT VALID to have two separate Mailbox
 *        instances--one Local and another remote-- that have the same Mailbox Name.
 *      - one for remote type Proxy Mailboxes for which the MailboxAddress and
 *        a reference/handle to the Proxy mailbox is stored.
 * - For non-existing/non-registered remote addresses (distributed, group, localSM),
 *   MailboxLookupService::find will create a proxy Mailbox to the remote address,
 *   activate it, and return a handle to it (handle allocated on heap).
 *   If there is a problem in creating the Proxy connection, find() may return NULL.
 * - For non-existing/non-registered local addresses (LocalMailbox),
 *   MailboxLookupService:: will return an error (consider how this can happen
 *   since all LocalMailboxes should be within the same process).
 * - Note that because of how the MailboxLookupService stores the Local and Remote
 *   type mailboxes, it is NOT VALID to have two separate Mailbox instances--one Local
 *   and another remote-- that have the same Mailbox Name.
 * <p>
 * - The DiscoverManager maintains a GroupMailbox that uses a multicast channel to
 *   communicate between all MailboxLookupService instances. This GroupMailbox is
 *   started with lazy initialization whenever a remote type Mailbox is registered/
 *   deregistered with the Lookup Service, or when a find on a remote type mailbox
 *   is performed.
 * - Any time that a remote type (Distributed, Group, LocalSM) Non-proxy Mailbox
 *   is Registered or Deregistered with the LookupService, a DiscoveryMessage is
 *   sent out to all other MLS instances that gives the Address and the type of
 *   operation (register/deregister).
 * - If an application needs to get notifications for when a new remote type Mailbox
 *   becomes available, it can perform registerForDiscoveryUpdates and pass in a
 *   reference to a MailboxAddress to match incoming DiscoveryMessages against and
 *   a MailboxOwnerHandle. The MailboxAddress to match against should ONLY have the 
 *   fields set to Non-Default values that the DiscoveryManager should consider in the 
 *   comparison with each incoming DiscoveryMessage. (This means that the developer
 *   should leave all fields defaulted that should NOT be used for comparison matching)
 * - Since, the application will typically need to re-determine all of its running
 *   mates upon initialization and restart, the registerForDiscoveryUpdates() method
 *   will return a sequence of all of the currently registered MailboxAddresses that
 *   match the search criteria.
 * - If a DiscoveryMessage comes in that matches the registered MailboxAddress search
 *   criteria, then a copy of the DiscoveryMessage will be posted to the registered
 *   MailboxOwnerHandle.
 * - If a MailboxOwnerHandle is deActivated and deregistered with the MLS, then
 *   it will be removed from the registry for DiscoveryUpdates. If it is then later
 *   re-activated, it will need to register for updates again.
 * <p>
 * The complete and proper handling for posting a message to a remote type Mailbox
 * is as follows (NOTE: This can be simplified for Local Mailboxes, and to some 
 * degree LocalSMMailboxes, as they are connectionless and do not require redundancy
 * retry semantics):
 *
 *    // Create the Test Message to post (on the heap)
 *    TestRemoteMessage* testRemoteMessage = new TestRemoteMessage();
 *
 *    // First test to make sure that we have a valid mailbox handle
 *    if (!testMailbox)
 *    {
 *       testMailbox = MailboxLookupService::find(testMailboxAddress);
 *    }//end if
 *
 *    // Post the Test Remote Message to the Test Mailbox
 *    if (testMailbox)
 *    {
 *       if (testMailbox->post(testRemoteMessage) == OK)
 *       {
 *          TRACELOG(DEBUGLOG, MSGMGRLOG, "Posted a remote message",0,0,0,0,0,0);
 *       }//end if
 *       else
 *       {
 *          // Since initial posting failed, we delete the handle, get a new handle from the MLS
 *          // and attempt re-post
 *          delete testMailbox;
 *          if (!(testMailbox = MailboxLookupService::find(testMailboxAddress)))
 *          {
 *             // Here, find failed so, the connection is bad and the handle ultimately
 *             // needs to be deleted and retried again.
 *             TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to perform MLS re-find after message post failed",0,0,0,0,0,0);
 *             delete testRemoteMessage;
 *          }//end else
 *          else if (testMailbox->post(testRemoteMessage) == ERROR)
 *          {
 *             TRACELOG(ERRORLOG, MSGMGRLOG, "Message post failed even after MLS re-find performed",0,0,0,0,0,0);
 *             delete testRemoteMessage;
 *          }//end else if
 *          else
 *          {
 *             TRACELOG(WARNINGLOG, MSGMGRLOG, "Initial message post failed, but worked after MLS re-find",0,0,0,0,0,0);
 *          }//end else
 *       }//end else
 *    }//end if
 *    else
 *    {
 *       TRACELOG(ERRORLOG, MSGMGRLOG, "Message post failed; Could not find mailbox",0,0,0,0,0,0);
 *    }//end else
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class DiscoveryManager;
class MailboxBase;
class MailboxHandle;
class MailboxOwnerHandle;
class MessageBase;

class MailboxLookupService
{
   /** A map to implement the registry for Local Mailboxes */
   typedef map<MailboxAddress, MailboxBase*, less<MailboxAddress> > LocalMailboxRegistry;

   /**
    * A map to implement the registry for Remote type Proxy Mailboxes 
    */
   typedef map<MailboxAddress, MailboxBase*, less<MailboxAddress> > ProxyMailboxRegistry;

   public:

      /** Virtual Destructor */
      virtual ~MailboxLookupService();

      /** Instantiate and start the Discovery Manager */
      static void initializeDiscoveryManager();

      /** Allows tasks to register their mailbox */
      static void registerMailbox(MailboxOwnerHandle* mailboxOwnerHandle, MailboxBase* mailboxPtr);

      /** Allows tasks to deregister their mailbox */
      static void deregisterMailbox(MailboxOwnerHandle* mailboxOwnerHandle);

      /** 
       * Allows an application to lookup and get a handle to another specific mailbox.
       * IMPORTANT: Here the returned MailboxHandle pointer is allocated on the heap.
       * It is the responsibility of the Applications to delete the MailboxHandle when
       * it is no longer needed (and to prevent a memory leak).
       * <p>
       * When the requested address is of LocationType LOCAL_MAILBOX, then the
       * the returned handle will point to a Local Mailbox (and posted messages
       * will be handled by the Local Mailbox post routine). If the requested
       * address is of a remote LocationType (Distributed, LocalSM, or Group),
       * then the returned handle will point to a Proxy Mailbox (and posted
       * messages will be serialized by the Proxy Mailbox's post routine for
       * remote delivery).
       */
      static MailboxHandle *find(const MailboxAddress& address);

      /**
       * Allows mailboxes to register for Discovery Update notifications.
       * If a DiscoveryMessage update is received whose discovery Mailbox
       * Address matches that of the matchCriteria, then a copy of the
       * DiscoveryMessage is posted to the mailboxToNotify. The caller is
       * required to pre-allocate a vector of MailboxAddresses which will be
       * populated with 'already-discovered' MailboxAddresses and returned.
       * <p>
       * Note that current there is no support for a user-initiated 
       * deregistration mechanism. This is because the discovery update
       * mechanism should not be used as a substitute for IPC between nodes.
       * <p>
       * @param matchCriteria MailboxAddress object used to determine which 
       *   discovery Mailbox Addresses match what the application wants to be
       *   notified about. NOTE: The calling application should leave all fields
       *   default except those that the caller wished to be used in the match
       *   comparison. For example, if the caller wants to be notified when 
       *   any Mailboxes with the MailboxName 'CallProcessor' are discovered,
       *   the matchCriteria field 'MailboxName' will be set to 'CallProcessor'
       *   and all other fields left as the default.
       * @param mailboxToNotify MailboxOwnerHandle of the mailbox that will
       *   receive the DiscoveryMessage notification when an update is received.
       *   This mailbox must provide a MessageHandler method for the Discovery
       *   Message id (MSGMGR_DISCOVERY_MSG_ID)
       * @returns OK upon successful registration; otherwise ERROR
       */
      static int registerForDiscoveryUpdates(vector<MailboxAddress>& currentlyRegisteredAddresses,
         MailboxAddress& matchCriteria, MailboxOwnerHandle* mailboxToNotify);

      /**
       * Display a list of all the registered mailbox addresses
       * This method displays its output in trace logs.
       */
      static void listAllMailboxAddresses();

      /** Method to set debug value for all mailboxes */
      static void setDebugForAllMailboxAddresses(int debugValue);

      /** Method to set debug value for all mailboxes */
      static void getDebugForAllMailboxAddresses();

   protected:

   private:

      /** Start the Discovery Manager dedicated thread */
      static void startDiscoveryManager();

      /**
       * NOTE: This method must be made private as it contradicts the framework
       * security rule that only the creator of a Mailbox (and the MailboxLookupService)
       * can get access to another mailbox's owner handle.
       * <p>
       * Allows an application to lookup and get a handle with owner privileges
       * to another specific mailbox. IMPORTANT: Here the returned MailboxOwnerHandle
       * pointer is allocated on the heap. It is the responsibility of the Applications
       * to delete the MailboxOwnerHandle when it is no longer needed (and to prevent
       * a memory leak). This method is limited to the owning application of the Mailbox.
       * <p>
       * When the requested address is of LocationType LOCAL_MAILBOX, then the
       * the returned handle will point to a Local Mailbox (and posted messages
       * will be handled by the Local Mailbox post routine). If the requested
       * address is of a remote LocationType (Distributed, LocalSM, or Group),
       * then the returned handle will point to a Proxy Mailbox (and posted
       * messages will be serialized by the Proxy Mailbox's post routine for
       * remote delivery).
       */
      static MailboxOwnerHandle *findOwnerHandle(const MailboxAddress& address);

      /** Constructor */
      MailboxLookupService();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MailboxLookupService(const MailboxLookupService& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MailboxLookupService& operator= (const MailboxLookupService& rhs);

      /** Registry member */
      static LocalMailboxRegistry localMailboxRegistry_;

      /** Non-recursive Mutex that controls insertion into the registry */
      static ACE_Thread_Mutex localRegistryMutex_;

      /** Registry for remote type Proxy Mailboxes (DistributedProxy, GroupProxy, LocalSMProxy) */
      static ProxyMailboxRegistry proxyMailboxRegistry_;

      /** Non-recursive Mutex that controls insertion into the registry */
      static ACE_Thread_Mutex proxyRegistryMutex_;

      /** Flag to indicate whether or not the Discovery Manager has been started yet */
      static bool isDiscoveryStarted_;

      /** Static instance of the Discovery Manager */
      static DiscoveryManager* discoveryManagerInstance_;

};

#endif
