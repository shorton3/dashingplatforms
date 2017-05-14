/******************************************************************************
*
* File name:   MailboxLookupService.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DiscoveryManager.h"  
#include "DistributedMailboxProxy.h"
#include "GroupMailboxProxy.h"
#include "LocalSMMailboxProxy.h"
#include "MailboxBase.h"
#include "MailboxLookupService.h"
#include "MailboxOwnerHandle.h"
#include "MailboxHandle.h"

#include "platform/logger/Logger.h"

#include "platform/common/MailboxNames.h"

#include "platform/threadmgr/ThreadManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Map for Local Mailbox registration
MailboxLookupService::LocalMailboxRegistry MailboxLookupService::localMailboxRegistry_;

// Non-Recursive Thread Mutex for protecting local registration map
ACE_Thread_Mutex MailboxLookupService::localRegistryMutex_;

// Map for Remote type Proxy Mailbox registration
MailboxLookupService::ProxyMailboxRegistry MailboxLookupService::proxyMailboxRegistry_;

// Non-Recursive Thread Mutex for protecting proxy registration map
ACE_Thread_Mutex MailboxLookupService::proxyRegistryMutex_;

// Flag to indicate whether or not the Discovery Manager group mailbox processor
// has been initialized
bool MailboxLookupService::isDiscoveryStarted_ = false;

// Static instance of the DiscoveryManager
DiscoveryManager* MailboxLookupService::discoveryManagerInstance_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxLookupService::MailboxLookupService()
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxLookupService::~MailboxLookupService()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Instantiate and start the Discovery Manager
// Design:
//-----------------------------------------------------------------------------
void MailboxLookupService::initializeDiscoveryManager()
{
   if ((isDiscoveryStarted_ == true) || (discoveryManagerInstance_ != NULL))
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Discovery Manager already started",0,0,0,0,0,0);
      return;
   }//end if
   else
   {
      // Instantiate the static Discovery Manager
      discoveryManagerInstance_ = new DiscoveryManager();

      // Mark the flag to indicate Discovery is started
      isDiscoveryStarted_ = true;

      if (discoveryManagerInstance_->initialize() == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to initialize the discovery manager",0,0,0,0,0,0);
         delete discoveryManagerInstance_;
         discoveryManagerInstance_ = NULL;
         isDiscoveryStarted_ = false;
         return;
      }//end if

      // Spawn a dedicated thread for the Discovery Manager
      ThreadManager::createThread((ACE_THR_FUNC)MailboxLookupService::startDiscoveryManager,
         0, "DiscoveryManager", true);
   }//end else 
}//end initializeDiscoveryManager


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method allows tasks to register their mailboxes with the
//              lookup service. If a task has already registered their mailbox
//              it will be erased first and the new one registered in it's place.
// Design:  MailboxLookupService maintains three Mailbox registries (actually, a
//          4th registry is used for discovery update notification callbacks):
//          - one for LocalMailboxes for which the MailboxAddress and a reference/
//              handle to the actual mailbox is stored. These are for local
//              applications to perform a 'find' on and post messages to.
//          - one for remote type Non-Proxy Mailboxes for which ONLY the Mailbox
//              Address is stored. These addresses are tracked across cards via
//              the DiscoveryManager. Since remote type non-proxy mailboxes
//              also have an underlying Local Mailbox implementation as well,
//              their local Mailbox equivalent gets stored in the LocalMailbox
//              registry (for the case where a Local, same process application
//              wishes to post a message to a Distributed Mailbox's Local Mailbox
//              queue--without going through message serialization/deserialization).
//              NOTE: For this reason, it is NOT VALID to have two separate Mailbox
//              instances--one Local and another remote-- that have the same Mailbox Name.
//          - one for remote type Proxy Mailboxes for which the MailboxAddress and
//              a reference/handle to the Proxy mailbox is stored.
//-----------------------------------------------------------------------------
void MailboxLookupService::registerMailbox(MailboxOwnerHandle* mailboxOwnerHandle,
                                           MailboxBase *mailboxPtr)
{
   // Pointer to an old Mailbox that is getting replaced in the registry in the event
   // that a duplicate is found
   MailboxBase* mailboxToReplace = NULL;

   if (mailboxOwnerHandle == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Null OwnerHandle passed to registerMailbox",0,0,0,0,0,0);
      return;
   }//end if
   else if (mailboxPtr == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Null mailbox pointer passed to registerMailbox",0,0,0,0,0,0);
      return;
   }//end else if

   //##############
   //# Local Registration
   //##############

   // Perform registration for Local Mailboxes
   if (mailboxOwnerHandle->getMailboxAddress().locationType == LOCAL_MAILBOX)
   {
      localRegistryMutex_.acquire();

      // Here we use a regular iterator (rather than const_iterator) since const_iterator cannot
      // be used for insert/erase (see Scott Meyers effective STL)
      LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
      LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();
      pair<LocalMailboxRegistry::iterator, bool> insertResult;

      // Loop through the map looking for the Mailbox that matches the address. This is a 
      // check to see if the Mailbox has already been registered and we need to replace.
      while (mailboxIterator != endIterator)
      {
         if (mailboxOwnerHandle->getMailboxAddress() == mailboxIterator->first)
         {
            ostringstream ostr;
            ostr << "Deactivating/Replacing the following local registry entry in the Lookup Service: "
                 << mailboxOwnerHandle->getMailboxAddress().toString() << ends;
            STRACELOG(WARNINGLOG, MSGMGRLOG, ostr.str().c_str());

            if (mailboxIterator->second->isActive())
            {
               // Set a flag to later call deactivate on the Mailbox because all
               // activated mailboxes must be in the registry!!
               mailboxToReplace =  mailboxIterator->second;
            }//end if
            else
            {
               // Found the corresponding Mailbox Address, so remove the Mailbox so we can replace it.
               localMailboxRegistry_.erase(mailboxIterator);
            }//end if
            break;
         }//end if
         mailboxIterator++;      
      }//end while

      // For new registrations
      if (mailboxIterator == endIterator)
      {
         ostringstream ostr;
         ostr << "Registering the following Local Address with the MLS: "
              << mailboxOwnerHandle->getMailboxAddress().toString() << ends;
         STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
      }//end if

      // After releasing the mutex on the local Registry, if we found the replaced mailbox to be
      // active, then we must deactivate it (which will ultimately perform the deregistration/erase for us)
      if (mailboxToReplace != NULL)
      {
         // Temporarily release the localMailboxRegistry mutex so that deactivate can perform
         // deregistration (This is needed since the mutex does NOT support recursive calls--we could use Recursive Mutex)
         localRegistryMutex_.release();
         mailboxToReplace->deactivate(mailboxOwnerHandle);
         mailboxToReplace = NULL;
         // Re-acquire so that we can insert the new value into the registry
         localRegistryMutex_.acquire();
      }//end if

      // Add the new mailbox to the registry (works for replacing existing entry or new)
      // Now, insert the mailbox/address into the map. This default 'insert' method is the only
      // one that provides a return/result value; otherwise, we would specify to insert at the end (endIterator)
      insertResult = localMailboxRegistry_.insert(make_pair(mailboxOwnerHandle->getMailboxAddress(),mailboxPtr));
      // Check to see if the map insert was successful
      if (!insertResult.second)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "LocalMailboxRegistry map insertion failed",0,0,0,0,0,0);   
      }//end if

      localRegistryMutex_.release();
   }//end if LocalMailbox
   // Perform registration for all other (Remote) Mailbox types
   else
   {
      //##############
      //# Non-Proxy Registration
      //##############

      // If this is a Non-proxy remote type Mailbox, then we need to register an equivalent
      // Local Mailbox address into the local registry (for the case where a Local,
      // same process application wishes to post a message to a Distributed Mailbox's
      // Local Mailbox queue--without going through message serialization/deserialization
      if (! mailboxPtr->isProxy())
      {
         localRegistryMutex_.acquire();

         // Here we use a regular iterator (rather than const_iterator) since const_iterator cannot
         // be used for insert/erase (see Scott Meyers effective STL)
         LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
         LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();
         pair<LocalMailboxRegistry::iterator, bool> insertResult;

         // Build the equivalent Local Mailbox Address for the remote type Mailbox that
         // we are attempting to register. NOTE: here Currently the Local Mailbox Addresses
         // really only consist of the location type and the Mailbox Name.
         MailboxAddress localAddress;
         localAddress.locationType = LOCAL_MAILBOX;
         localAddress.mailboxName = mailboxOwnerHandle->getMailboxAddress().mailboxName;

         // Loop through the map looping for the Mailbox that matches the address. This is a
         // check to see if the Mailbox has already been registered and we need to replace. This HERE is
         // the reason why a Local Mailbox and Remote Mailbox CANNOT have the same MailboxName!!
         while (mailboxIterator != endIterator)
         {
            if (localAddress == mailboxIterator->first)
            {
               ostringstream ostr;
               ostr << "Deactivating/Replacing the following local (remote-equivalent) registry entry in the Lookup Service: "
                    << localAddress.toString() << ends;
               STRACELOG(WARNINGLOG, MSGMGRLOG, ostr.str().c_str());

               if (mailboxIterator->second->isActive())
               {
                  // Set a flag to later call deactivate on the Mailbox because all
                  // activated mailboxes must be in the registry!!
                  mailboxToReplace =  mailboxIterator->second;
               }//end if
               else
               {
                  // Found the corresponding Mailbox Address, so remove the Mailbox so we can replace it.
                  localMailboxRegistry_.erase(mailboxIterator);
               }//end if
               break;
            }//end if
            mailboxIterator++;
         }//end while

         // For new registrations
         if (mailboxIterator == endIterator)
         {
            ostringstream ostr;
            ostr << "Registered the following Local Address (remote-equivalent) with the MLS: "
                 << localAddress.toString() << ends;
            STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
         }//end if

         // After releasing the mutex on the local Registry, if we found the replaced mailbox to be
         // active, then we must deactivate it (which will ultimately perform the deregistration/erase for us)
         if (mailboxToReplace != NULL)
         {
            // Temporarily release the localMailboxRegistry mutex so that deactivate can perform
            // deregistration (This is needed since the mutex does NOT support recursive calls; we could use recursive mutex)
            localRegistryMutex_.release();
            mailboxToReplace->deactivate(mailboxOwnerHandle);
            mailboxToReplace = NULL;
            // Re-acquire so that we can insert the new value into the registry
            localRegistryMutex_.acquire();
         }//end if

         // Add the new mailbox to the registry (works for replace existing entry or new)
         // Now, insert the mailbox/address into the map. This default 'insert' method is the only
         // one that provides a return/result value; otherwise, we would specify to insert at the end (endIterator)
         insertResult = localMailboxRegistry_.insert(make_pair(localAddress,mailboxPtr));
         // Check to see if the map insert was successful
         if (!insertResult.second)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "LocalMailboxRegistry map insertion failed for remote-equivalent",0,0,0,0,0,0);
         }//end if

         localRegistryMutex_.release();

         // Add the non-proxy remote Address to the Discovery Manager registry and send Discovery updates
         if (isDiscoveryStarted_)
         {
            discoveryManagerInstance_->registerLocalAddress(mailboxOwnerHandle->getMailboxAddress());
         }//end if
      }//end if

      //##############
      //# Proxy Registration
      //##############

      // Else, for Proxy Mailboxes, register the handler in the proxy registry
      else
      {
         // Note that we do not allow the Discovery Manager Proxy Mailbox to register with
         // the MLS. We need this check here since the GroupMailboxProxy will regardless try
         // to perform registration. We will force block it here.
         if (strcmp (mailboxOwnerHandle->getMailboxAddress().mailboxName.c_str(), DISCOVERY_MANAGER_MAILBOX_NAME) == 0)
         {
            TRACELOG(DEVELOPERLOG, MSGMGRLOG, "MLS disallows Discovery Manager Proxy Mailbox to register",0,0,0,0,0,0);
            return;
         }//end if
 
         proxyRegistryMutex_.acquire();

         // Here we use a regular iterator (rather than const_iterator) since const_iterator cannot
         // be used for insert/erase (see Scott Meyers effective STL)
         ProxyMailboxRegistry::iterator mailboxIterator = proxyMailboxRegistry_.begin();
         ProxyMailboxRegistry::iterator endIterator = proxyMailboxRegistry_.end();
         pair<ProxyMailboxRegistry::iterator, bool> insertResult;

         // Loop through the map looping for the Mailbox that matches the address. This is a
         // check to see if the Mailbox has already been registered and we need to replace.
         while (mailboxIterator != endIterator)
         {
            if (mailboxOwnerHandle->getMailboxAddress() == mailboxIterator->first)
            {
               ostringstream ostr;
               ostr << "Deactivating/Replacing the following proxy registry entry in the Lookup Service: "
                    << mailboxOwnerHandle->getMailboxAddress().toString() << ends;
               STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

               if (mailboxIterator->second->isActive())
               { 
                  // Set a flag to later call deactivate on the Mailbox because all
                  // activated mailboxes must be in the registry!!
                  mailboxToReplace =  mailboxIterator->second;
               }//end if
               else
               {
                  // Found the corresponding Mailbox Address, so remove the Mailbox so we can replace it.
                  proxyMailboxRegistry_.erase(mailboxIterator);
               }//end if
               break;
            }//end if
            mailboxIterator++;
         }//end while

         // For new registrations
         if (mailboxIterator == endIterator)
         {
            ostringstream ostr;
            ostr << "Registered the following proxy registry entry with the MLS: "
                 << mailboxOwnerHandle->getMailboxAddress().toString() << ends;
            STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
         }//end if

         // After releasing the mutex on the proxy Registry, if we found the replaced mailbox to be
         // active, then we must deactivate it (which will ultimately perform the deregistration/erase for us)
         if (mailboxToReplace != NULL)
         {
            // Temporarily release the proxyMailboxRegistry mutex so that deactivate can perform
            // deregistration (This is needed since the mutex supports recursive calls; we could use recursive mutex)
            proxyRegistryMutex_.release(); 
            mailboxToReplace->deactivate(mailboxOwnerHandle);
            mailboxToReplace = NULL;
            // Re-acquire so that we can insert the new value into the registry
            proxyRegistryMutex_.acquire();
         }//end if

         // Add the new mailbox to the registry (works for replace existing entry or new)
         // Now, insert the mailbox/address into the map. This default 'insert' method is the only
         // one that provides a return/result value; otherwise, we would specify to insert at the end (endIterator)
         insertResult = proxyMailboxRegistry_.insert(make_pair(mailboxOwnerHandle->getMailboxAddress(),mailboxPtr));
         // Check to see if the map insert was successful
         if (!insertResult.second)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "ProxyMailboxRegistry map insertion failed",0,0,0,0,0,0);
         }//end if

         proxyRegistryMutex_.release();
      }//end else
   }//end else

   // For debugging
   if (Logger::getSubsystemLogLevel(MSGMGRLOG) == DEVELOPERLOG)
   {
      listAllMailboxAddresses();
   }//end if
}//end registerMailbox


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method allows tasks to deregister their mailbox. The lookup
//              service will remove the mailbox entry from the various registries. 
// Design:
//-----------------------------------------------------------------------------
void MailboxLookupService::deregisterMailbox(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (mailboxOwnerHandle == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Null OwnerHandle passed to deregisterMailbox",0,0,0,0,0,0);
      return;
   }//end if

   //##############
   //# Local DeRegistration
   //##############

   // Perform deregistration for Local Mailboxes
   if (mailboxOwnerHandle->getMailboxAddress().locationType == LOCAL_MAILBOX)
   {
      localRegistryMutex_.acquire();

      LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
      LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();

      // Loop through the map looking for a LocalMailbox that matches the address.
      while (mailboxIterator != endIterator)
      {
         if (mailboxOwnerHandle->getMailboxAddress() == mailboxIterator->first)
         {
            ostringstream ostr;
            ostr << "Deregistering the following Local Address with the MLS: " 
                 << ((MailboxAddress)mailboxIterator->first).toString() << ends;
            STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

            // Here we don't call deactivate since it could call deregister again if the
            // mailbox is already activated. We depend on the applications to call 
            // deactivate BEFORE/IF they call deregister

            // Found the corresponding Mailbox Address, so remove the Mailbox
            localMailboxRegistry_.erase(mailboxIterator);
            break;
         }//end if
         mailboxIterator++;
      }//end while

      localRegistryMutex_.release();

      // Check to see if this remote type Non-Proxy Mailbox was registered to receive
      // discovery update notifications. If so, we need to deregister it. We do this here
      // because there is currently no limitation that requires Mailboxes to be remote
      // type to receive discovery update messages (they can be Local or Remote Non-Proxy,
      // but they of course cannot be Proxy)
      if (isDiscoveryStarted_)
      {
         discoveryManagerInstance_->deregisterForDiscoveryUpdates(mailboxOwnerHandle);
      }//end if
   }//end if
   // Perform deregistration for remote type Mailboxes
   else
   {
      //##############
      //# Non-Proxy DeRegistration
      //##############

      // If this is a Non-proxy remote type Mailbox, then we need to deregister the equivalent
      // Local Mailbox address from the local registry
      if (! mailboxOwnerHandle->isProxy())
      {
         localRegistryMutex_.acquire();
                                                                                                                              
         // Here we use a regular iterator (rather than const_iterator) since const_iterator cannot
         // be used for insert/erase (see Scott Meyers effective STL)
         LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
         LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();

         // Build the equivalent Local Mailbox Address for the remote type Non-proxy Mailbox that
         // we are attempting to deregister. NOTE: here Currently the Local Mailbox Addresses
         // really only consist of the location type and the Mailbox Name.
         MailboxAddress localAddress;
         localAddress.locationType = LOCAL_MAILBOX;
         localAddress.mailboxName = mailboxOwnerHandle->getMailboxAddress().mailboxName;

         // Loop through the map looking for the Mailbox that matches the address
         while (mailboxIterator != endIterator)
         {
            if (localAddress == mailboxIterator->first)
            {
               ostringstream ostr;
               ostr << "Deregistering the following Local Address (remote-equivalent) with the MLS: "
                    << ((MailboxAddress)mailboxIterator->first).toString() << ends;
               STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

               // Here we don't call deactivate since it could call deregister again if the
               // mailbox is already activated. We depend on the applications to call
               // deactivate BEFORE/IF they call deregister

               // Found the corresponding Mailbox Address, so remove the Mailbox
               localMailboxRegistry_.erase(mailboxIterator);
               break;
            }//end if
            mailboxIterator++;
         }//end while

         localRegistryMutex_.release();

         // Remove the non-proxy remote Address from the Discovery Manager registry and send Discovery updates
         if (isDiscoveryStarted_)
         {
            // First see if the Mailbox had registered for Discovery updates and deregister those
            discoveryManagerInstance_->deregisterForDiscoveryUpdates(mailboxOwnerHandle);
            // Then deregister the mailbox from Discovery
            discoveryManagerInstance_->deregisterLocalAddress(mailboxOwnerHandle->getMailboxAddress());
         }//end if
      }//end if !proxy

      //##############
      //# Proxy Registration
      //##############

      // Else, for Proxy Mailboxes, deregister the handler from the proxy registry
      else
      {
         proxyRegistryMutex_.acquire();

         ProxyMailboxRegistry::iterator mailboxIterator = proxyMailboxRegistry_.begin();
         ProxyMailboxRegistry::iterator endIterator = proxyMailboxRegistry_.end();

         // Loop through the map looping for the Mailbox that matches the address
         while (mailboxIterator != endIterator)
         {
            if (mailboxOwnerHandle->getMailboxAddress() == mailboxIterator->first)
            {
               ostringstream ostr;
               ostr << "Deregistering the following Proxy Address with the MLS: "
                    << ((MailboxAddress)mailboxIterator->first).toString() << ends;
               STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

               // Here we don't call deactivate since it could call deregister again if the
               // mailbox is already activated. We depend on the applications to call
               // deactivate BEFORE/IF they call deregister

               // Found the corresponding Mailbox Address, so remove the Mailbox
               proxyMailboxRegistry_.erase(mailboxIterator);
               break;
            }//end if
            mailboxIterator++;
         }//end while
         proxyRegistryMutex_.release();
      }//end else
   }//end else

   // For debugging
   if (Logger::getSubsystemLogLevel(MSGMGRLOG) == DEVELOPERLOG)
   {
      listAllMailboxAddresses();
   }//end if   
}//end deregisterMailbox


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method allows tasks to find other task mailboxes. If the
//              requested task mailbox exists it creates a handle to the mailbox,
//              acquires it (done in its constructor) and then returns it.
//              Otherwise it returns a NULL.
// Design:      When the requested address is of LocationType LOCAL_MAILBOX, then
//              the returned handle will point to a Local Mailbox (and posted
//              messages will be handled by the Local Mailbox post routine). If
//              the requested address is of a remote LocationType (Distributed,
//              LocalSM, or Group), then the returned handle will point to a Proxy
//              Mailbox (and posted messages will be serialized by the Proxy
//              Mailbox's post routine for remote delivery).
//               
//              References to Non-Proxy Remote type Mailboxes DO exist
//              in the MailboxLookupService, but in the nonProxyRemote registry. They are
//              not returned by the find() or findOwner() methods because they are
//              not used by the applications for posting messages (Local Mailbox handles
//              are used for posting local messages; Proxy Mailbox handles are
//              used for posting remote type messages). Non-Proxy Remote type
//              Mailboxes exist in the LookupService for Discovery purposes only.
//              The flow here is that if an application needs to find all of the
//              mailboxes of type X, then it will perform a registerForDiscoveryUpdates
//              method which will return a sequence of matching addresses.
//              Then, if the application needs to communicate with those mailboxes,
//              it will perform a 'find' which will create the proxy mailbox
//              connection to that remote mailbox.
//-----------------------------------------------------------------------------
MailboxHandle* MailboxLookupService::find(const MailboxAddress& address)
{
   MailboxHandle* mailboxHandlePtr = NULL;

   if (address.locationType == UNKNOWN_MAILBOX_LOCATION)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Attempting to perform find with MailboxAddress LocationType UNKNOWN",0,0,0,0,0,0);
      return mailboxHandlePtr;
   }//end if

   // Find Local type mailboxes
   else if (address.locationType == LOCAL_MAILBOX)
   {
      localRegistryMutex_.acquire();

      LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
      LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();

      // Loop through the map looking for the Mailbox that matches the address
      while (mailboxIterator != endIterator)
      {
         if ((MailboxAddress)address == mailboxIterator->first)
         {
            if (Logger::getSubsystemLogLevel(MSGMGRLOG) == DEVELOPERLOG)
            {
               ostringstream ostr;
               ostr << "Found the following Local Address in the MLS: "
                    << ((MailboxAddress)mailboxIterator->first).toString() << ends;
               STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
            }//end if

            // Found the corresponding Mailbox Address, so return a handle to it
            // HERE, we are allocating memory for the handle on the heap, so it is upto the
            // application developer to release this memory (Applications OWN the handles)
            mailboxHandlePtr = new MailboxHandle((*mailboxIterator).second);
            break;
         }//end if
         mailboxIterator++;
      }//end while

      // If we cannot find the entry for a LocalMailbox in the registry, then that's an ERROR 
      // (since LocalMailbox should be within the same process as the caller)
      if (mailboxIterator == endIterator)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot find entry in MLS registry for LocalMailbox",0,0,0,0,0,0);
      }//end if

      localRegistryMutex_.release();
   }//end if
   // For remote type mailboxes
   else
   {
      proxyRegistryMutex_.acquire();

      ProxyMailboxRegistry::iterator mailboxIterator = proxyMailboxRegistry_.begin();
      ProxyMailboxRegistry::iterator endIterator = proxyMailboxRegistry_.end();

      // Loop through the map looking for the Mailbox that matches the address
      while (mailboxIterator != endIterator)
      {
         if ((MailboxAddress)address == mailboxIterator->first)
         {
            if (Logger::getSubsystemLogLevel(MSGMGRLOG) == DEVELOPERLOG)
            {
               ostringstream ostr;
               ostr << "Found the following Remote Address for Proxy Mailbox in the MLS: "
                    << ((MailboxAddress)mailboxIterator->first).toString() << ends;
               STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
            }//end if

            // Found the corresponding Mailbox Address, so return a handle to it
            // HERE, we are allocating memory for the handle on the heap, so it is upto the
            // application developer to release this memory (Applications OWN the handles)
            mailboxHandlePtr = new MailboxHandle((*mailboxIterator).second);
            break;
         }//end if
         mailboxIterator++;
      }//end while

      // If we cannot find the entry for a Remote type Mailbox in the registry, then we attempt to 
      // create a Proxy Mailbox connection to it.
      if (mailboxIterator == endIterator)
      {
         TRACELOG(DEBUGLOG, MSGMGRLOG, "Cannot find entry in MLS registry for Remote type mailbox, attempting proxy connect",0,0,0,0,0,0);

         // Release the lock on the remote registry before we attempt to activate/register any of
         // the below proxy mailboxes (this is to prevent deadlock)
         proxyRegistryMutex_.release(); 

         // Create the proxy mailbox
         MailboxOwnerHandle* proxyOwnerHandle = NULL;
         if (address.locationType == LOCAL_SHARED_MEMORY_MAILBOX)
         {
            proxyOwnerHandle = LocalSMMailboxProxy::createMailbox(address);
         }//end if
         else if (address.locationType == DISTRIBUTED_MAILBOX)
         {
            proxyOwnerHandle = DistributedMailboxProxy::createMailbox(address);
         }//end else if
         else if (address.locationType == GROUP_MAILBOX)
         {
            proxyOwnerHandle = GroupMailboxProxy::createMailbox(address);
         }//end else if
         else
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to find mailbox handle with illegal location type (%d)",
               address.locationType,0,0,0,0,0);
            return NULL;
         }//end else

         // Perform activate to initiate the connection and register this proxy with the lookup service
         if (proxyOwnerHandle->activate() == ERROR)
         {
            TRACELOG(WARNINGLOG, MSGMGRLOG, "Error activating proxy mailbox, application must retry the find operation",0,0,0,0,0,0);
            return NULL;
         }//end if
         else
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "Successfully activated proxy mailbox",0,0,0,0,0,0);
         }//end else

         // Set the proxy's owner handle to a 'regular' handle type so we can process the location type
         mailboxHandlePtr = proxyOwnerHandle->generateMailboxHandleCopy();

         // Delete the proxy's owner handle so that we only rely on the Application's proxy handle to
         // to keep the proxy mailbox alive (as long as they don't delete and cause a 'release')
         delete proxyOwnerHandle;
      }//end if
      else
      {
         // Release the lock on the remote registry
         proxyRegistryMutex_.release();
      }//end else
   }//end else

   // Log the proxy creation or type of mailbox handle returned
   if (mailboxHandlePtr != NULL)
   {
      if (mailboxHandlePtr->getMailboxAddress().locationType == LOCAL_MAILBOX)
      {
         TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning handle to Local Mailbox",0,0,0,0,0,0);
      }//end if
      else if (mailboxHandlePtr->getMailboxAddress().locationType == DISTRIBUTED_MAILBOX)
      {
         if (mailboxHandlePtr->isProxy())
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning handle to Distributed Mailbox Proxy",0,0,0,0,0,0);
         }//endif
         else
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning handle to Local Equivalent of Distributed Mailbox",0,0,0,0,0,0);
         }//end if
      }//end else if
      else if (mailboxHandlePtr->getMailboxAddress().locationType == LOCAL_SHARED_MEMORY_MAILBOX)
      {
         if (mailboxHandlePtr->isProxy())
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning handle to Local Shared Memory Mailbox Proxy",0,0,0,0,0,0);
         }//end if
         else
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning handle to Local Equivalent of Local Shared Memory Mailbox",0,0,0,0,0,0);
         }//end else
      }//end else if
      else if (mailboxHandlePtr->getMailboxAddress().locationType == GROUP_MAILBOX)
      {
         if (mailboxHandlePtr->isProxy())
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning handle to Group Mailbox Proxy",0,0,0,0,0,0);
         }//end if
         else
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning handle to Local Equivalent of Group Mailbox",0,0,0,0,0,0);
         }//end else
      }//end else if
   }//end if
   return mailboxHandlePtr;
}//end find


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allows mailboxes to register for Discovery Update notifications
// Design:
//-----------------------------------------------------------------------------
int MailboxLookupService::registerForDiscoveryUpdates(vector<MailboxAddress>& currentlyRegisteredAddresses,
   MailboxAddress& matchCriteria, MailboxOwnerHandle* mailboxToNotify)
{
   if (isDiscoveryStarted_)
   {
      return discoveryManagerInstance_->registerForDiscoveryUpdates(currentlyRegisteredAddresses,
         matchCriteria, mailboxToNotify);
   }//end if
   return ERROR;
}//end registerForDiscoveryUpdates


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method outputs all registered Mailbox Addresses.
// Design: Here, we put the results into 3 separate Log Messages to avoid 
//   truncation.
//-----------------------------------------------------------------------------
void MailboxLookupService::listAllMailboxAddresses()
{
   ostringstream ostr;

   // Display Local Mailboxes:
   localRegistryMutex_.acquire();
   ostr << "List of currently registered local mailboxes (" << localMailboxRegistry_.size() << "):" << endl;

   LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
   LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();

   // Loop through the map
   while (mailboxIterator != endIterator)
   {
      ostr << "|" << ((MailboxAddress)mailboxIterator->first).toString() << "|" << endl;
      mailboxIterator++;
   }//end while 
   localRegistryMutex_.release();

   // Log can get very big. May be truncated
   STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

   // Display Remote Proxy Mailboxes:
   ostringstream ostr2;
   proxyRegistryMutex_.acquire();
   ostr2 << "List of currently registered remote proxy mailboxes (" << proxyMailboxRegistry_.size() << "):" << endl;

   ProxyMailboxRegistry::iterator rMailboxIterator = proxyMailboxRegistry_.begin();
   ProxyMailboxRegistry::iterator rEndIterator = proxyMailboxRegistry_.end();

   // Loop through the map
   while (rMailboxIterator != rEndIterator)
   {
      ostr2 << "|" << ((MailboxAddress)rMailboxIterator->first).toString() << "|" << endl;
      rMailboxIterator++;
   }//end while
   proxyRegistryMutex_.release();

   // Log can get very big. May be truncated
   STRACELOG(DEBUGLOG, MSGMGRLOG, ostr2.str().c_str()); 
   
   // Display Remote (via Discovery) and Locally registered Non-Proxy remote type Mailboxes:
   if (isDiscoveryStarted_)
   {
      discoveryManagerInstance_->listAllMailboxAddresses();
   }//end if
}//end listAllMailboxAddresses


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Set debug flags for all mailboxes
// Design: No need to do anything to the nonProxy set since that is only Addresses
//   and no real references to mailboxes
//-----------------------------------------------------------------------------
void MailboxLookupService::setDebugForAllMailboxAddresses(int debugValue)
{
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Setting Debug Flag for All Mailboxes to %d",debugValue, 0,0,0,0,0);

   // Set the local mailboxes
   localRegistryMutex_.acquire();

   LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
   LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();

   // Loop through the map 
   while (mailboxIterator != endIterator)
   {
      (mailboxIterator->second)->setDebugValue(debugValue);
      mailboxIterator++;
   }//end while

   localRegistryMutex_.release();
 
   // Set the remote proxy mailboxes
   proxyRegistryMutex_.acquire();

   ProxyMailboxRegistry::iterator rMailboxIterator = proxyMailboxRegistry_.begin();
   ProxyMailboxRegistry::iterator rEndIterator = proxyMailboxRegistry_.end();

   // Loop through the map
   while (rMailboxIterator != rEndIterator)
   {
      (rMailboxIterator->second)->setDebugValue(debugValue);
      rMailboxIterator++;
   }//end while

   proxyRegistryMutex_.release();
}//end setDebugForAllMailboxAddresses


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Display debug flags for all mailboxes
// Design: No need to do anything to the nonProxy set since that is only Addresses
//   and no real references to mailboxes
//-----------------------------------------------------------------------------
void MailboxLookupService::getDebugForAllMailboxAddresses()
{
   ostringstream ostr;
   ostr << "Debug values for all mailboxes: " << endl;

   // Local Mailboxes
   localRegistryMutex_.acquire(); 

   LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
   LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();

   // Loop through the map looping for the Mailbox that matches the address
   while (mailboxIterator != endIterator)
   {
      ostr << ((MailboxAddress)mailboxIterator->first).toString() << "-------->"
           << (mailboxIterator->second)->getDebugValue() << endl;
      mailboxIterator++;
   }//end while

   localRegistryMutex_.release();

   // Remote proxy Mailboxes
   proxyRegistryMutex_.acquire();

   ProxyMailboxRegistry::iterator rMailboxIterator = proxyMailboxRegistry_.begin();
   ProxyMailboxRegistry::iterator rEndIterator = proxyMailboxRegistry_.end();

   // Loop through the map looping for the Mailbox that matches the address
   while (rMailboxIterator != rEndIterator)
   {
      ostr << ((MailboxAddress)rMailboxIterator->first).toString() << "-------->"
           << (rMailboxIterator->second)->getDebugValue() << endl;
      rMailboxIterator++;
   }//end while

   proxyRegistryMutex_.release();
}//end getDebugForAllMailboxAddresses


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Start the Discovery Manager's mailbox processor in a dedicated
//    thread.
// Design:
//-----------------------------------------------------------------------------
void MailboxLookupService::startDiscoveryManager()
{
   // Start Discovery Manager's mailbox processing loop
   discoveryManagerInstance_->processMailbox();
}//end startDiscoveryManager


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method allows tasks to find other task mailboxes. If the
//              requested task mailbox exists it creates a handle to the mailbox
//              and returns it. Otherwise it returns a NULL. This handle has
//              owner privileges as opposed to the handle returned in the find()
// Design:      Here, Only return existing mailbox Owner handles (do not attempt
//              any connects). This method is PRIVATE so it should only be used
//              within the MailboxLookupService.
//-----------------------------------------------------------------------------
MailboxOwnerHandle* MailboxLookupService::findOwnerHandle(const MailboxAddress& address)
{
   MailboxOwnerHandle* mailboxHandlePtr = NULL;

   if (address.locationType == UNKNOWN_MAILBOX_LOCATION)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Attempting to perform find with MailboxAddress LocationType UNKNOWN",0,0,0,0,0,0);
      return mailboxHandlePtr;
   }//end if

   // Find Local type mailboxes
   else if (address.locationType == LOCAL_MAILBOX)
   {
      localRegistryMutex_.acquire();

      LocalMailboxRegistry::iterator mailboxIterator = localMailboxRegistry_.begin();
      LocalMailboxRegistry::iterator endIterator = localMailboxRegistry_.end();

      // Loop through the map looping for the Mailbox that matches the address
      while (mailboxIterator != endIterator)
      {
         if ((MailboxAddress)address == mailboxIterator->first)
         {
            if (Logger::getSubsystemLogLevel(MSGMGRLOG) == DEVELOPERLOG)
            {
               ostringstream ostr;
               ostr << "Found the following Local Address in the MLS: "
                    << ((MailboxAddress)mailboxIterator->first).toString() << ends;
               STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
            }//end if

            // Found the corresponding Mailbox Address, so return a handle to it
            mailboxHandlePtr = new MailboxOwnerHandle((*mailboxIterator).second);
            break;
         }//end if
         mailboxIterator++;
      }//end while

      // If we cannot find the entry for a LocalMailbox in the registry.
      if (mailboxIterator == endIterator)
      {
         TRACELOG(WARNINGLOG, MSGMGRLOG, "Cannot find entry in MLS registry for LocalMailbox",0,0,0,0,0,0);
      }//end if

      localRegistryMutex_.release();
   }//end if
   // For remote type mailboxes
   else
   {
      proxyRegistryMutex_.acquire();

      ProxyMailboxRegistry::iterator mailboxIterator = proxyMailboxRegistry_.begin();
      ProxyMailboxRegistry::iterator endIterator = proxyMailboxRegistry_.end();

      // Loop through the map looping for the Mailbox that matches the address
      while (mailboxIterator != endIterator)
      {
         if ((MailboxAddress)address == mailboxIterator->first)
         {
            if (Logger::getSubsystemLogLevel(MSGMGRLOG) == DEVELOPERLOG)
            {
               ostringstream ostr;
               ostr << "Found the following Remote Address for Proxy Mailbox in the MLS: "
                    << ((MailboxAddress)mailboxIterator->first).toString() << ends;
               STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
            }//end if

            // Found the corresponding Mailbox Address, so return a handle to it
            mailboxHandlePtr = new MailboxOwnerHandle((*mailboxIterator).second);
            break;
         }//end if
         mailboxIterator++;
      }//end while

      // If we cannot find the entry for a Remote type Mailbox in the registry
      if (mailboxIterator == endIterator)
      {
         TRACELOG(WARNINGLOG, MSGMGRLOG, "Cannot find entry in MLS registry for Remote type Proxy Mailbox",0,0,0,0,0,0);
      }//end if
      proxyRegistryMutex_.release();
   }//end else

   // Log the type of mailbox handle returned
   if (mailboxHandlePtr != NULL)
   {
      if (mailboxHandlePtr->getMailboxAddress().locationType == LOCAL_MAILBOX)
      {
         TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning owner handle to Local Mailbox",0,0,0,0,0,0);
      }//end if
      else if (mailboxHandlePtr->getMailboxAddress().locationType == DISTRIBUTED_MAILBOX)
      {
         if (mailboxHandlePtr->isProxy())
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning owner handle to Distributed Mailbox Proxy",0,0,0,0,0,0);
         }//endif
         else
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning owner handle to Local Equivalent of Distributed Mailbox",0,0,0,0,0,0);
         }//end if
      }//end else if
      else if (mailboxHandlePtr->getMailboxAddress().locationType == LOCAL_SHARED_MEMORY_MAILBOX)
      {
         if (mailboxHandlePtr->isProxy())
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning owner handle to Local Shared Memory Mailbox Proxy",0,0,0,0,0,0);
         }//end if
         else
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning owner handle to Local Equivalent of Local Shared Memory Mailbox",0,0,0,0,0,0);
         }//end else
      }//end else if
      else if (mailboxHandlePtr->getMailboxAddress().locationType == GROUP_MAILBOX)
      {
         if (mailboxHandlePtr->isProxy())
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning owner handle to Group Mailbox Proxy",0,0,0,0,0,0);
         }//end if
         else
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "MLS returning owner handle to Local Equivalent of Group Mailbox",0,0,0,0,0,0);
         }//end else
      }//end else if
   }//end if

   // If we cannot find the entry for a Remote type Mailbox in the registry, then return NULL 
   return mailboxHandlePtr;
}//end findOwnerHandle


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

