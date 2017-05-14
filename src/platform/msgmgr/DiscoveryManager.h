/******************************************************************************
* 
* File name:   DiscoveryManager.h 
* Subsystem:   Platform Services 
* Description: Works with MailboxLookupService to communicate the registrations/
*              deregistrations of remote type Non-Proxy mailboxes across cards.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DISCOVERY_MANAGER_H_
#define _PLAT_DISCOVERY_MANAGER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <map>  // For multimap
#include <set>
#include <vector>

#include <ace/Thread_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DiscoveryMessage.h"
#include "MailboxAddress.h"

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
 * DiscoveryManager works with MailboxLookupService to communicate the registrations/
 * deregistrations of remote type Non-Proxy mailboxes across cards.
 * <p>
 * DiscoveryManager provides a necessary function for applications to be notified
 * dynamically at runtime when new system nodes/instances come online.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MailboxOwnerHandle;
class MailboxProcessor;
class MessageHandlerList;

class DiscoveryManager
{
   /**
    * An STL set of Non-Proxy remote type Mailbox Addresses (for Discovery tracking)
    */
   typedef set<MailboxAddress> NonProxyRegistry;

   /**
    * A multimap for storing the registrations of Mailbox's that wish to receive Discovery Update notifications
    */
   typedef multimap<MailboxAddress, MailboxOwnerHandle*, less<MailboxAddress> > DiscoveryUpdateRegistry;

   public:

      /** Constructor */
      DiscoveryManager();

      /** Virtual Destructor */
      virtual ~DiscoveryManager();

      /** 
       * Perform initialization of Discovery Manager which communicates mailbox
       * updates between distributed nodes.
       * @returns OK upon success; otherwise ERROR
       */
      int initialize();

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
      int registerForDiscoveryUpdates(vector<MailboxAddress>& currentlyRegisteredAddresses,
         MailboxAddress& matchCriteria, MailboxOwnerHandle* mailboxToNotify);

      /**
       * Allows mailboxes to deregister for Discovery Update notifications
       * @return OK upon success; otherwise ERROR
       */
      int deregisterForDiscoveryUpdates(MailboxOwnerHandle* mailboxToNotify);

      /**
       * Add local registration of a remote type Non-Proxy Mailbox Address.
       * This will facilitate a discovery REGISTER update being sent to all other DiscoveryManagers
       * @returns OK upon successful registration; otherwise ERROR
       */
      int registerLocalAddress(MailboxAddress& localRemoteTypeAddress);

      /** 
       * Remove the local registration of a remote type Non-Proxy MailboxAddress.
       * This will facilitate a discovery DEREGISTER update being sent to all other DiscoveryManagers.
       * @returns OK upon successful registration; otherwise ERROR
       */
      int deregisterLocalAddress(MailboxAddress& localRemoteTypeAddress);

      /**
       * Display a list of all the registered remote type Non-Proxy mailbox addresses
       * This method displays its output in trace logs.
       */
      void listAllMailboxAddresses();

      /** Start the mailbox processing loop */
      void processMailbox();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      DiscoveryManager(const DiscoveryManager& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DiscoveryManager& operator= (const DiscoveryManager& rhs);

      /** Create the Discovery Manager proxy mailbox (also used to re-create it) */
      int createDiscoveryManagerProxy();

      /**
       * Message Handler for Discovery Group Messages
       * @param message base class object passed by the Functor
       * @returns OK if successful; otherwise ERROR
       */
      int processDiscoveryMessage(MessageBase* message);

      /** 
       * Message Handler for Local Discovery Messages (only passed through the Local Mailbox)
       * @param message base class object passed by the Functor
       * @returns OK if successful; otherwise ERROR
       */
      int processLocalDiscoveryMessage(MessageBase* message);

      /**
       * Convenience method to post Discovery Message Updates
       * @returns OK if successful; otherwise ERROR
       */
      int postDiscoveryMessage(DiscoveryOperationType operationType, const MailboxAddress& addressToSend);

      /** Registry for remote type Non-Proxy Mailboxes */
      NonProxyRegistry nonProxyMailboxRegistry_;

      /**  
       * Non-Recursive Mutex that controls insertion/deletion from the non proxy registry.
       * We would not need this since we are doing synchronous processing of operations;
       * however, the applications need the capability to synchronously register for 
       * discovery updates, so then protection is required.
       */
      ACE_Thread_Mutex nonProxyRegistryMutex_;

      /** Registry for Mailbox's that wish to receive Discovery Update notifications */
      DiscoveryUpdateRegistry discoveryUpdateRegistry_;

      /** Non-Recursive Mutex that controls access to the discovery update registry */
      ACE_Thread_Mutex discoveryUpdateRegistryMutex_;

      /** Discovery Manager Group Mailbox Address */
      MailboxAddress discoveryManagerAddress_;

      /** Discovery Manager Proxy Mailbox used for posting Group Messages to the other Discovery Managers */
      MailboxOwnerHandle* discoveryManagerProxyMailbox_;

      /** Discovery Manager Group Mailbox */
      MailboxOwnerHandle* discoveryManagerMailbox_;

      /** Message Handler List for storing Functors for Mailbox handlers */
      MessageHandlerList* messageHandlerList_;

      /** Mailbox Processor */
      MailboxProcessor* mailboxProcessor_;

      /** Process PID for this DiscoveryManager instance */
      unsigned int localPID_;

};

#endif
