/******************************************************************************
*
* File name:   DistributedMailbox.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Condition_Thread_Mutex.h>
#include <ace/Select_Reactor.h>
#include <ace/Thread.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DistributedMailbox.h"
#include "MailboxOwnerHandle.h"
#include "MessageFactory.h"

#include "platform/logger/Logger.h"

#include "platform/threadmgr/ThreadManager.h"

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
DistributedMailbox::DistributedMailbox(const MailboxAddress& distributedAddress)
                                       : LocalMailbox(distributedAddress), /* Base class */
                                         distributedAddress_ (distributedAddress),
                                         socketAcceptor_ (NULL),
                                         messageBuffer_ (MAX_MESSAGE_LENGTH),
                                         distributedReactor_ (NULL)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DistributedMailbox::~DistributedMailbox()
{
   // Flag that we are shutting down
   isShuttingDown_ = TRUE;

   distributedReactor_->end_reactor_event_loop();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Activate the distributed mailbox
// Design:
//-----------------------------------------------------------------------------
int DistributedMailbox::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (isActive())
   {
      return OK;
   }//end if

   // Begin listening for socket IO
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Distributed Mailbox activate is called",0,0,0,0,0,0);

   // Base class will start the Reactor event loop in a separate thread and
   // handle mailbox registration. We need to perform this PRIOR to opening the
   // group mailbox receiver socket, since we will need the ability to post as soon
   // as we do that.
   if (LocalMailbox::activate(mailboxOwnerHandle) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to active Local Mailbox Base for Distributed Mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // For first-time activation, Create the acceptor instance. This also calls
   // the Reactor Singleton which creates the Reactor for the first time.
   if (socketAcceptor_ == NULL)
   {
      // Pass the address to accept/listen on, also specify the REUSE_ADDR flag '1'
      socketAcceptor_ = new ACE_SOCK_Acceptor (distributedAddress_.inetAddress, 1); 

      // Register this instance's handle_input method with the reactor
      if (distributedReactor_->register_handler(socketAcceptor_->get_handle(), this, ACE_Event_Handler::READ_MASK) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Register handler failed",0,0,0,0,0,0);
         // IF this occurs, we need to Deactivate the LocalMailbox base class since that will de-register
         // from the MailboxLookupService
         LocalMailbox::deactivate(mailboxOwnerHandle);
         return ERROR;
      }//end if
   }//end if
   else
   {
      // Since this mailbox has been previously deactivated, we just need to
      // open the Acceptor socket
      if (socketAcceptor_->open(distributedAddress_.inetAddress, 1) == ERROR)
      {
         char errorBuff[200];
         char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (resultStr == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if
         ostringstream ostr;
         ostr << "Failed to open socket Acceptor with errno (" << resultStr << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

         // IF this occurs, we need to Deactivate the LocalMailbox base class since that will de-register
         // from the MailboxLookupService
         LocalMailbox::deactivate(mailboxOwnerHandle);
         return ERROR;
      }//end if
   }//end else

   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Deactivate the distributed mailbox
// Design:
//-----------------------------------------------------------------------------
int DistributedMailbox::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Distributed mailbox deactivate is called",0,0,0,0,0,0);

   // Close the listener socket
   socketAcceptor_->close();

   // Base class will end the Reactor processing loop
   return (LocalMailbox::deactivate(mailboxOwnerHandle));
}//end deactivate
                                                                                                           

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Rename the remote address for the mailbox
// Design:
//-----------------------------------------------------------------------------
bool DistributedMailbox::rename(const MailboxAddress& newRemoteAddress)
{
   // For now, do a dummy assignment to prevent the compiler from giving a warning
   const MailboxAddress tmpAddress __attribute__ ((unused)) = newRemoteAddress;

   TRACELOG(ERRORLOG, MSGMGRLOG, "Incomplete implementation for rename called",0,0,0,0,0,0);
   return true;
}//end rename


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allows applications to create a mailbox and get a handle to it.
// Design:
//-----------------------------------------------------------------------------
MailboxOwnerHandle* DistributedMailbox::createMailbox(const MailboxAddress& distributedAddress)
{
   // Make sure that we attempting to create the appropriate mailbox type
   if (distributedAddress.locationType != DISTRIBUTED_MAILBOX)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Attempting to create DistributedMailbox, but Address locationtype is %d",
         distributedAddress.locationType,0,0,0,0,0);
      return NULL;
   }//end if

   DistributedMailbox* distributedMailbox = new DistributedMailbox(distributedAddress);
   if (!distributedMailbox)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create a distributed mailbox",0,0,0,0,0,0);
      return NULL;
   }//end if

   MailboxOwnerHandle* mailboxOwnerHandle = new MailboxOwnerHandle(distributedMailbox);
   if (!mailboxOwnerHandle) 
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create an owner handle to distributed mailbox",0,0,0,0,0,0);
   }//end else

   // Instantiate the ACE_Select_Reactor used by the LocalMailbox for Timers
   distributedMailbox->selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create a new ACE_Select_Reactor for the message handling, etc to use
   distributedMailbox->distributedReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Start the Reactor Event Loop -- used for Timer processing. This has to be
   // started in a separate thread. Here, returns OS assigned unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)LocalMailbox::startReactor,
      (void*)distributedMailbox->selectReactor_, "LocalMailboxReactor", true);

   // Start the Reactor Event Loop used for socket IO handling
   ThreadManager::createThread((ACE_THR_FUNC)DistributedMailbox::startReactor,
      (void*)distributedMailbox->distributedReactor_, "DistributedMailboxReactor", true);

   return mailboxOwnerHandle;
}//end createMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description:  Overriden ACE_Event_Handler method. Called back automatically 
//               when a connection has been established and upon receiving data
//               (by the Reactor framework)
// Design:
//-----------------------------------------------------------------------------
int DistributedMailbox::handle_input(ACE_HANDLE handle)
{
   // For the case where handle_input is called for connection establishment,
   // the ACE_HANDLE parameter passed in should be the handle for the Acceptor's
   // listener socket.
   if (handle == socketAcceptor_->get_handle())
   {
      // From ACE docs:  The <ACE_SOCK_Acceptor> has its own "passive-mode" socket.
      // This serves as a factory to create so-called "data-mode"
      // sockets, which are what the <ACE_SOCK_Stream> encapsulates.
      // Therefore, by inheriting from <ACE_SOCK>, <ACE_SOCK_Acceptor>
      // gets its very own socket.
      // So here, we are creating data-mode sockets by creating a new SockStream
      // object to perform accept() on:
      ACE_SOCK_Stream* newSockStream = new ACE_SOCK_Stream();
      if (socketAcceptor_->accept(*newSockStream) == ERROR)
      {
         char errorBuff[200];
         char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (resultStr == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if
         ostringstream ostr;
         ostr << "Failed performing accept on distributed mailbox errno (" << resultStr << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());
         return ERROR;
      }//end if

      // Insert the new sock stream into our map based on its newly created ACE_HANDLE
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Distributed Mailbox storing new connection client",0,0,0,0,0,0);

      clientConnectorMapMutex_.acquire();
      pair<ClientConnectorMap::iterator, bool> insertResult;
      insertResult = clientConnectorMap_.insert(make_pair(newSockStream->get_handle(), newSockStream));
      // Check to see if the map insert was successful
      if (!insertResult.second)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "ClientConnectorMap insertion failed",0,0,0,0,0,0);
      }//end if
      clientConnectorMapMutex_.release();

      // Register this class object with the reactor as the ACE_Event_Handler for
      // receiving messages on this data-mode socket.
      if (distributedReactor_->register_handler(newSockStream->get_handle(), this, ACE_Event_Handler::READ_MASK) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Register handler for data mode socket failed",0,0,0,0,0,0);
      }//end if
   }//end if
   // For the case where handle_input is called for receiving data on one of the
   // 'mapped' data-mode sockets, we need to receive the data and enqueue it to the
   // underlying local mailbox for processing
   else
   {
      // Retrieve the associated ACE_SOCK_Stream object for the passed-in ACE_HANDLE
      // file descriptor (from our mapping)
      clientConnectorMapMutex_.acquire();
      ACE_SOCK_Stream* receivingSockStream = clientConnectorMap_[handle];

      // Receive the data into our message buffer
      int numberBytes = 0;
      if ((numberBytes = receivingSockStream->recv(messageBuffer_.getBuffer(), MAX_MESSAGE_LENGTH)) <= 0)
      {
         char errorBuff[200];
         char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (resultStr == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if
         ostringstream ostr;
         ostr << "Recv failed on distributed mailbox with return value (" << numberBytes
              << ") and errno (" << resultStr << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());
 
         // Tell the reactor not to handle events for this handle anymore
         if (distributedReactor_->remove_handler(handle, ACE_Event_Handler::READ_MASK) == ERROR)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Remove handler failed",0,0,0,0,0,0);
         }//end if

         // if recv returns 0, assume that the communication is broken so unregister this socket
         // from the Map, delete the associated Sock Stream, and return
         clientConnectorMap_.erase(handle);
         clientConnectorMapMutex_.release();
         delete receivingSockStream;

         TRACELOG(WARNINGLOG, MSGMGRLOG, "Detected lost connection for distributed mailbox",0,0,0,0,0,0);
         // Clear the buffer for the next loop iteration
         messageBuffer_.clearBuffer();
         return OK;
      }//end if
      else
      {
         clientConnectorMapMutex_.release();
      }//end else

      // Set the insertion pointer for our Message Buffer
      messageBuffer_.setInsertPosition(numberBytes);

      // Perform Message Id specific deserialization of the buffer back into a MessageBase type
      MessageBase* message = MessageFactory::recreateMessageFromBuffer(messageBuffer_);
      if (message != NULL)
      {
         // Deserialize the Message Version Number - DO NOT DO AUTOMATIC SERIALIZATION OF VERSION...
         // BUT LEAVE THIS CODE AS EXAMPLE OF HOW TO EMBED/SERIALIZE/DESERIALIZE HIDEN/AUTOMATIC PARMS
         //unsigned int versionNumber = 0;
         //messageBuffer_ >> versionNumber;
         //message->setVersion(versionNumber);
 
         // First check to see if the messageBuffer is now empty, if not, we need to deserialize
         // additional flags such as the priorityLevel flag
         if (!messageBuffer_.areContentsProcessed())
         {
            unsigned int messagePriorityLevel = 0;
            messageBuffer_ >> messagePriorityLevel;
            message->setPriority(messagePriorityLevel);
         }//end if 

         if (debugValue_)
         {
            ostringstream debugMsg;
            char tmpBuffer[30];
            char tmpBuffer2[30];
            message->getSourceAddress().inetAddress.addr_to_string(tmpBuffer, sizeof(tmpBuffer));
            distributedAddress_.inetAddress.addr_to_string(tmpBuffer2, sizeof(tmpBuffer2));
            debugMsg << "##RECEIVING MESSAGE## " <<
                        " SOURCE_ADDRESS>> " << tmpBuffer << 
                        " DESTINATION_ADDRESS>> " << tmpBuffer2 << 
                        " MESSAGE_ID>> 0x" << hex << message->getMessageId() << 
                        " MESSAGE_CONTENT>> " << message->toString() << ends;
            STRACELOG(DEBUGLOG, MSGMGRLOG, debugMsg.str().c_str());
         }//end if

         incrementReceivedCount();

         // if deserialization was successful, post the new message to our local mailbox 
         if (post(message) == ERROR)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error enqueuing a received distributed message to mailbox",0,0,0,0,0,0);
         }//end if
      }//end if

      // Clear the buffer for the next loop iteration
      messageBuffer_.clearBuffer();
   }//end else
   return OK;
}//end handle_input


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overriden ACE_Event_Handler method. Called back automatically
//              when a connection has been dropped
// Design:
//-----------------------------------------------------------------------------
int DistributedMailbox::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
   // dummy declarations to prevent unused variable compiler warnings
   ACE_HANDLE dummyHandle __attribute__ ((unused)) = handle;
   ACE_Reactor_Mask mask __attribute__ ((unused)) = close_mask;

   TRACELOG(DEBUGLOG, MSGMGRLOG, "ACE Event Handler handle_close called",0,0,0,0,0,0);
   return OK;
}//end handle_close


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string DistributedMailbox::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox distributed address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& DistributedMailbox::getMailboxAddress()
{
   return distributedAddress_;
}//end getMailboxAddress

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method starts the reactor thread needed for processing
//              send/receive messages, etc.
// Design:
//-----------------------------------------------------------------------------
void DistributedMailbox::startReactor(void* arg)
{
   // Convert the void* arg to a Reactor pointer
   ACE_Reactor* reactor = static_cast<ACE_Reactor*>(arg); 

   // Set Reactor thread ownership
   reactor->owner (ACE_Thread::self ());
   // Start the reactor processing loop
   while ((isShuttingDown_ == FALSE) && (reactor->reactor_event_loop_done () == 0))
   {
      int result = reactor->run_reactor_event_loop ();

      char errorBuff[200];
      char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
      if (resultStr == NULL)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Distributed reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

      // Perform reset on the reactor
      reactor->reset_reactor_event_loop();
   }//end while

   // Need to do something to keep ThreadManager from restarting it
   //if (isShuttingDown_)
   //{
   //   ACE_Thread_Manager::instance()->at_exit(NULL,0);
   //}//end if
}//end startReactor


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

