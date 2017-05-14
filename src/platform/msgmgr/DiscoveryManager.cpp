/******************************************************************************
*
* File name:   DiscoveryManager.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DiscoveryManager.h"
#include "DiscoveryLocalMessage.h"
#include "DiscoveryMessage.h"
#include "GroupMailbox.h"
#include "GroupMailboxProxy.h"
#include "MailboxOwnerHandle.h"
#include "MessageBase.h"
#include "MessageFactory.h"
#include "MessageHandlerList.h"
#include "MailboxProcessor.h"

#include "platform/logger/Logger.h"

#include "platform/common/MailboxNames.h"
#include "platform/common/MessageIds.h"

#include "platform/utilities/SystemInfo.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DiscoveryManager::DiscoveryManager()
   : discoveryManagerProxyMailbox_ (NULL),
     mailboxProcessor_ (NULL),
     localPID_ (0)
{
   // Create the Discovery Manager Mailbox Address (do this before we spawn the thread in order
   // to prevent a race condition with the register/deregister methods above)
   discoveryManagerAddress_.locationType = GROUP_MAILBOX;
   discoveryManagerAddress_.mailboxName = DISCOVERY_MANAGER_MAILBOX_NAME;
   discoveryManagerAddress_.inetAddress.set((short unsigned int)DISCOVERY_MANAGER_MAILBOX_PORT,
      DISCOVERY_MANAGER_IP_ADDRESS);
   discoveryManagerAddress_.neid = SystemInfo::getLocalNEID();
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DiscoveryManager::~DiscoveryManager()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform mailbox initialization, etc
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::initialize()
{
   // Store our local process PID
   localPID_ = getpid();

   // Create the Local Mailbox
   discoveryManagerMailbox_ = GroupMailbox::createMailbox(discoveryManagerAddress_);
   if (!discoveryManagerMailbox_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create Discovery Manager group mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Create the Proxy Mailbox for posting messages
   if (createDiscoveryManagerProxy() == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create Discovery Manager proxy mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create message handler list",0,0,0,0,0,0);
      delete discoveryManagerMailbox_;
      return ERROR;
   }//end if

   // Create handlers for each message expected
   MessageHandler discoveryMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &DiscoveryManager::processDiscoveryMessage);

   MessageHandler discoveryLocalMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &DiscoveryManager::processLocalDiscoveryMessage);

   // Add the message handlers to the message handler list to be used by the MsgMgr
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor
   messageHandlerList_->add(MSGMGR_DISCOVERY_MSG_ID, discoveryMessageHandler);
   messageHandlerList_->add(MSGMGR_DISCOVERY_LOCAL_MSG_ID, discoveryLocalMessageHandler);

   // Register support for group messages so that the MessageFactory
   // knows how to recreate them in 'MessageBase' form when they are received.
   MessageBootStrapMethod discoveryMessageBootStrapMethod = makeFunctor( (MessageBootStrapMethod*)0,
                                               DiscoveryMessage::deserialize);
   MessageFactory::registerSupport(MSGMGR_DISCOVERY_MSG_ID, discoveryMessageBootStrapMethod);

   // Here, we go ahead a create the Mailbox Processor and activate the Mailbox. We need do
   // this here instead of in the processMailbox() method because we want to post messages
   // into the queue before the processing begins (since Discovery happens so early).
   mailboxProcessor_ = new MailboxProcessor(messageHandlerList_, *discoveryManagerMailbox_);
                                                                                                                               
   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (discoveryManagerMailbox_->activate() == ERROR)
   {
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the group side. Applications need to retry.
      TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to activate discovery manager group mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Add the local registration of a remote type Non-Proxy MailboxAddress
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::registerLocalAddress(MailboxAddress& localRemoteTypeAddress)
{
   // Create a local discovery message for the register operation
   DiscoveryLocalMessage* discoveryLocalMessage = new DiscoveryLocalMessage(discoveryManagerAddress_,
      DISCOVERY_LOCAL_REGISTER, localRemoteTypeAddress);

   // Post a local message to the discovery Manager mailbox to perform the registration
   return discoveryManagerMailbox_->post (discoveryLocalMessage);
}//end registerLocalAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Remove the local registration of a remote type Non-Proxy MailboxAddress
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::deregisterLocalAddress(MailboxAddress& localRemoteTypeAddress)
{
   // Create a local discovery message for the register operation
   DiscoveryLocalMessage* discoveryLocalMessage = new DiscoveryLocalMessage(discoveryManagerAddress_,
      DISCOVERY_LOCAL_DEREGISTER, localRemoteTypeAddress);

   // Post a local message to the discovery Manager mailbox to perform the registration
   return discoveryManagerMailbox_->post (discoveryLocalMessage);
}//end deregisterLocalAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Allows mailboxes to register for Discovery Update notifications
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::registerForDiscoveryUpdates(vector<MailboxAddress>& currentlyRegisteredAddresses,
   MailboxAddress& matchCriteria, MailboxOwnerHandle* mailboxToNotify)
{
   bool foundDuplicateRegistration = false;
   discoveryUpdateRegistryMutex_.acquire();
   // Check to make sure that we have not registered the same matchCriteria and
   // mailboxToNotify already since this is a multimap. We could take this section
   // out later or make it dependent on some debug flag later to slightly increase performance
   //
   // We do this because, if there are duplicate entries for discovery update, then that
   // application's mailbox will receive multiple DiscoveryMessage notifications for
   // the same update
   DiscoveryUpdateRegistry::iterator discoveryRegIterator = discoveryUpdateRegistry_.begin();
   DiscoveryUpdateRegistry::iterator discoveryEndIterator = discoveryUpdateRegistry_.end();
   while (discoveryRegIterator != discoveryEndIterator)
   {
      // Here, since making copies of MailboxOwnerHandles is not allowed, it should
      // be safe to just do pointer comparison (should be the same object)
      if ((mailboxToNotify == discoveryRegIterator->second) && (matchCriteria == discoveryRegIterator->first))
      {
         ostringstream ostr;
         ostr << "Found duplicate entry when registering Mailbox for Discovery Updates at Address: "
              << discoveryRegIterator->first.toString() << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());
         foundDuplicateRegistration = true;
         // Do not break here since the mailbox may be registered multiple times (multimap)
      }//end if
   }//end while

   // If no duplicates were found, go ahead and add/insert the registration
   if (foundDuplicateRegistration == false)
   {
      // According to the STL, this insert into multimap should not fail
      discoveryUpdateRegistry_.insert(make_pair(matchCriteria, mailboxToNotify));
   }//end if
   discoveryUpdateRegistryMutex_.release();

   // Now loop through the nonProxy Remote Registry of MailboxAddresses and pick out all of
   // the addresses that compare successfully to the match Criteria, put each one in the
   // currentlyRegisteredAddresses vector so that they get returned to the calling application.
   nonProxyRegistryMutex_.acquire();
   NonProxyRegistry::iterator addressIterator = nonProxyMailboxRegistry_.begin();
   NonProxyRegistry::iterator endSetIterator = nonProxyMailboxRegistry_.end();
   while (addressIterator != endSetIterator)
   {
      if (MailboxAddress::isMatchingAddress(matchCriteria, *addressIterator))
      {
         // Just insert into the end of the vector (STL will make a copy of the Address)
         currentlyRegisteredAddresses.push_back(*addressIterator);
      }//end if
   }//end while
   nonProxyRegistryMutex_.release();
   return OK;
}//end registerForDiscoveryUpdates


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Allows mailboxes to deregister for Discovery Update notifications
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::deregisterForDiscoveryUpdates(MailboxOwnerHandle* mailboxToNotify)
{
   // Check to see if this remote type Non-Proxy Mailbox was registered to receive
   // discovery update notifications. If so, we need to deregister it. We do this here
   // because there is currently no limitation that requires Mailboxes to be remote
   // type to receive discovery update messages (they can be Local or Remote Non-Proxy,
   // but they of course cannot be Proxy)
   discoveryUpdateRegistryMutex_.acquire();

   DiscoveryUpdateRegistry::iterator discoveryRegIterator = discoveryUpdateRegistry_.begin();
   DiscoveryUpdateRegistry::iterator discoveryEndIterator = discoveryUpdateRegistry_.end();

   while (discoveryRegIterator != discoveryEndIterator)
   {
      // Here, since making copies of MailboxOwnerHandles is not allowed, it should
      // be safe to just do pointer comparison (should be the same object)
      if (mailboxToNotify == discoveryRegIterator->second)
      {
         ostringstream ostr;
         ostr << "Deregistering match criteria from receiving Discovery Update Notifications at Address: "
              << discoveryRegIterator->first.toString() << ends;
         STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
         // Found the discovery update registration, so remove it
         discoveryUpdateRegistry_.erase(discoveryRegIterator);
         // Do not break here since the mailbox may be registered for multiple match criteria (multimap)
      }//end if
   }//end while
   discoveryUpdateRegistryMutex_.release();
   return OK;
}//end deregisterForDiscoveryUpdates


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: List all registered remote type Non-Proxy mailbox addresses
// Design: Output is sent to trace logs
//-----------------------------------------------------------------------------
void DiscoveryManager::listAllMailboxAddresses()
{
   MailboxAddress dummyAddress;
   // Create a local discovery message for the register operation
   DiscoveryLocalMessage* discoveryLocalMessage = new DiscoveryLocalMessage(discoveryManagerAddress_,
      DISCOVERY_LOCAL_DISPLAY, dummyAddress);

   // Post a local message to the discovery Manager mailbox to perform the registration
   discoveryManagerMailbox_->post (discoveryLocalMessage);
}//end listAllMailboxAddresses


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Main mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void DiscoveryManager::processMailbox()
{
   // Start processing messages from the Mailbox queue
   mailboxProcessor_->processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Local Discovery Messages
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::processLocalDiscoveryMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   DiscoveryLocalMessage* discoveryMessage = (DiscoveryLocalMessage*) message;

   string str = "Received a Local Discovery Message: " + discoveryMessage->toString();
   STRACELOG(DEBUGLOG, MSGMGRLOG, str.c_str());

   // Remove the discovery local message contents from the remote non-proxy registry
   if (discoveryMessage->getOperationType() == DISCOVERY_LOCAL_DEREGISTER)
   {
      // Remove the non-proxy remote Address from the non-proxy remote registry set
      nonProxyRegistryMutex_.acquire();
      NonProxyRegistry::iterator addressIterator = nonProxyMailboxRegistry_.begin();
      NonProxyRegistry::iterator endSetIterator = nonProxyMailboxRegistry_.end();
      while (addressIterator != endSetIterator)
      {
         if (discoveryMessage->getDiscoveryAddress() == (*addressIterator))
         {
            ostringstream ostr;
            ostr << "Deregistering the following Non Proxy Address with the DiscoveryManager: "
                 << addressIterator->toString() << ends;
            STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
            nonProxyMailboxRegistry_.erase(addressIterator);
            break;
         }//end if
      }//end while
      nonProxyRegistryMutex_.release();

      // Post the remote deregistration to the Discovery Manager for sending to remote nodes
      postDiscoveryMessage(DISCOVERY_DEREGISTER, discoveryMessage->getDiscoveryAddress());
   }//end if
   // Store the discovery local message contents in the remote non-proxy registry
   else if (discoveryMessage->getOperationType() == DISCOVERY_LOCAL_REGISTER)
   {
       // Add the non-proxy remote Address to the non-proxy remote registry set. This is for
       // storing the addresses of all remote type mailboxes created here locally as well as
       // those remote type Mailbox addresses that we know about through Discovery
       nonProxyRegistryMutex_.acquire();
       pair<NonProxyRegistry::iterator, bool> insertSetResult;
       insertSetResult = nonProxyMailboxRegistry_.insert(discoveryMessage->getDiscoveryAddress());

       // Check to see if the set insert was successful
       if (!insertSetResult.second)
       {
          TRACELOG(ERRORLOG, MSGMGRLOG, "NonProxyMailboxRegistry set insertion failed",0,0,0,0,0,0);
       }//end if
       else
       {
          ostringstream ostr;
          ostr << "Registered the following Non Proxy Address with the DiscoveryManager: "
               << discoveryMessage->getDiscoveryAddress().toString() << ends;
          STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
       }//end else
       nonProxyRegistryMutex_.release();

       // Now post the non-proxy remote Address to the Discovery Manager so that all MLS will see it
       postDiscoveryMessage(DISCOVERY_REGISTER, discoveryMessage->getDiscoveryAddress());
postDiscoveryMessage(DISCOVERY_REGISTER, discoveryMessage->getDiscoveryAddress());
postDiscoveryMessage(DISCOVERY_REGISTER, discoveryMessage->getDiscoveryAddress());
   }//end else if
   // Display the contents of the remote non-proxy registry (in trace log)
   else if (discoveryMessage->getOperationType() == DISCOVERY_LOCAL_DISPLAY)
   {
      // Display Remote (via Discovery) and Locally registered Non-Proxy remote type Mailboxes:
      ostringstream ostr3;
      nonProxyRegistryMutex_.acquire();
      ostr3 << "List of currently registered non-proxy remote type addresses, both locally registered and via Discovery ("
            << nonProxyMailboxRegistry_.size() << "):" << endl;

      NonProxyRegistry::iterator npMailboxIterator = nonProxyMailboxRegistry_.begin();
      NonProxyRegistry::iterator npEndIterator = nonProxyMailboxRegistry_.end();

      // Loop through the set
      while (npMailboxIterator != npEndIterator)
      {
         ostr3 << "|" << npMailboxIterator->toString() << "|" << endl;
         npMailboxIterator++;
      }//end while
      nonProxyRegistryMutex_.release();

      //WARNING!! This log can get very BIG!! It will be truncated in that case.
      STRACELOG(DEBUGLOG, MSGMGRLOG, ostr3.str().c_str());
   }//end else if
   else
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unknown Discovery Local Operation type (%d)",
         discoveryMessage->getOperationType(),0,0,0,0,0);
      return ERROR;
   }//end else
   return OK;
}//end processLocalDiscoveryMessage

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Discovery Messages
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::processDiscoveryMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   DiscoveryMessage* discoveryMessage = (DiscoveryMessage*) message;
 
   // Here, we need to do a check to see if the message that we received on the
   // Discovery remote Group Mailbox was posted by OUR OWN Discovery PROXY MAILBOX
   // If it was from us, then we disregard it since we don't want to process it
   // twice and we don't want to send out notifications to all of the registered
   // applications twice.

   // Can we do this by embedding the pid in the DiscoveryMessage, so that we can
   // compare both the sourceAddress and the pid. We probably need to look at the pid
   // right?
   if ((discoveryMessage->getSourceAddress() == discoveryManagerAddress_) &&
       (discoveryMessage->getOriginatingPID() == localPID_))
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Discarding DiscoveryMessage from self",0,0,0,0,0,0);
      return OK;
   }//end if

   string str = "Received a Discovery Message: " + discoveryMessage->toString();
   STRACELOG(DEBUGLOG, MSGMGRLOG, str.c_str());

   // Remove the non-proxy remote Address from the non-proxy remote registry set
   if (discoveryMessage->getOperationType() == DISCOVERY_DEREGISTER)
   {
      nonProxyRegistryMutex_.acquire();
      NonProxyRegistry::iterator addressIterator = nonProxyMailboxRegistry_.begin();
      NonProxyRegistry::iterator endIterator = nonProxyMailboxRegistry_.end();
      while (addressIterator != endIterator)
      {
         if (discoveryMessage->getDiscoveryAddress() == (*addressIterator))
         {
            ostringstream ostr;
            ostr << "Discovery causing deregistration of the following Non Proxy Address with the MLS: "
                 << discoveryMessage->getDiscoveryAddress().toString() << ends;
            STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
            nonProxyMailboxRegistry_.erase(addressIterator);
            break;
         }//end if
      }//end while
      nonProxyRegistryMutex_.release();
   }//end if
   // Store the discovery message contents in the remote non-proxy registry
   else if (discoveryMessage->getOperationType() == DISCOVERY_REGISTER)
   {
      // Add the non-proxy remote Address to the non-proxy remote registry set
      nonProxyRegistryMutex_.acquire();
      pair<NonProxyRegistry::iterator, bool> insertResult;
      insertResult = nonProxyMailboxRegistry_.insert(discoveryMessage->getDiscoveryAddress());
      // Check to see if the set insert was successful
      if (!insertResult.second)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "NonProxyMailboxRegistry set insertion failed",0,0,0,0,0,0);
      }//end if
      else
      {
         ostringstream ostr;
         ostr << "Discovery causing registration of the following Non Proxy Address with the MLS: "
              << discoveryMessage->getDiscoveryAddress().toString() << ends;
         STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
      }//end else
      nonProxyRegistryMutex_.release();
   }//end else if
   else
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unknown Discovery Operation type (%d)",
         discoveryMessage->getOperationType(),0,0,0,0,0);
   }//end else

   // Now, loop through all of the discovery Update listeners and post a copy of the
   // DiscoveryMessage to each of their mailboxes
   discoveryUpdateRegistryMutex_.acquire();
   DiscoveryUpdateRegistry::iterator discoveryRegIterator = discoveryUpdateRegistry_.begin();
   DiscoveryUpdateRegistry::iterator discoveryEndIterator = discoveryUpdateRegistry_.end();
   while (discoveryRegIterator != discoveryEndIterator)
   {
      if (MailboxAddress::isMatchingAddress(discoveryRegIterator->first, discoveryMessage->getDiscoveryAddress()))
      {
         DiscoveryMessage* newDiscoveryMessage = new DiscoveryMessage(*discoveryMessage);
         discoveryRegIterator->second->post(newDiscoveryMessage);
      }//end if
   }//end while
   discoveryUpdateRegistryMutex_.release();

   return OK;
}//end processDiscoveryMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Convenience method to post Discovery Messages
// Design:  NOTE that Discovery Manager Proxy Mailbox does not exist in the
//   remote proxy registry
//-----------------------------------------------------------------------------
int DiscoveryManager::postDiscoveryMessage(DiscoveryOperationType operationType,
   const MailboxAddress& addressToSend)
{
   // Now post the non-proxy remote Address to the Discovery Manager so that all MLS will see it
   string operationStr("Unknown");
   if (operationType == DISCOVERY_REGISTER)
   {
      operationStr = "Register";
   }//end if
   else if (operationType == DISCOVERY_DEREGISTER)
   {
      operationStr = "Deregister";
   }//end else

   DiscoveryMessage* discoveryMessage = new DiscoveryMessage(discoveryManagerAddress_,
      operationType, localPID_, addressToSend);
   // First check to see if we have a valid handle
   if (!discoveryManagerProxyMailbox_)
   {
      createDiscoveryManagerProxy();
   }//end if

   // If we now have a valid handle, then post the message
   if (discoveryManagerProxyMailbox_)
   {
      // Remember that if post is successful, then the MsgMgr framework will delete it; otherwise,
      // it is the application's responsibility to delete it or retry
      if (discoveryManagerProxyMailbox_->post(discoveryMessage) != ERROR)
      {
         ostringstream ostr;
         ostr << "Posted MLS " << operationStr << " discovery message" << ends;
         STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
      }//end if
      else
      {
         delete discoveryManagerProxyMailbox_;
         if (createDiscoveryManagerProxy() == ERROR)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to re-create Discovery Manager Proxy",0,0,0,0,0,0);
            delete discoveryMessage;
            return ERROR;
         }//end else
         else if (discoveryManagerProxyMailbox_->post(discoveryMessage) == ERROR)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Message post failed even after Discovery Manager Proxy re-create performed",0,0,0,0,0,0);
            delete discoveryMessage;
            return ERROR;
         }//end else if
         else
         {
            ostringstream ostr;
            ostr << "Initial " << operationStr
                 << " discovery message post failed, but worked after Proxy re-create" << ends;
            STRACELOG(WARNINGLOG, MSGMGRLOG, ostr.str().c_str());
         }//end else
      }//end else
   }//end if
   else
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not create Discovery Manager Proxy",0,0,0,0,0,0);
      return ERROR;
   }//end else
   return OK;
}//end postDiscoveryMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Convenience method to create the Discovery Manager Proxy Mailbox
// Design:
//-----------------------------------------------------------------------------
int DiscoveryManager::createDiscoveryManagerProxy()
{
   // NOTE: Here we do NOT allow Discovery Manager's Group Mailbox Proxy to exist
   // in the remote Proxy registry in the MLS. This is an optimization, so we have
   // no need to search the registry
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Creating Discovery Manager Proxy",0,0,0,0,0,0);
   discoveryManagerProxyMailbox_ = GroupMailboxProxy::createMailbox(discoveryManagerAddress_, TRUE, 255);
   // Perform activate to initiate the connection
   if (discoveryManagerProxyMailbox_->activate() == ERROR)
   {
      TRACELOG(WARNINGLOG, MSGMGRLOG, "Error activating Discovery Manager Proxy",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Successfully activated Discovery Manager Proxy",0,0,0,0,0,0);
   }//end else

discoveryManagerProxyMailbox_->setDebugValue(10);


   return OK;
}//end createDiscoveryManagerProxy


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

