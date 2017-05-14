/******************************************************************************
*
* File name:   LocalSMMailbox.cpp
* Subsystem:   Platform Services
* Description: Mailbox class for receiving messages via Local Shared
*              Memory Queue. This mailbox type allows processes to exchange
*              messages via shared memory, thus avoiding the overhead
*              caused by the network stack. Note that a LocalMailbox is
*              transparently associated with this Mailbox type (which is necessary
*              to provide local-only access as well as concurrency for Timers).
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

#include <cstring>

#include <ace/Process_Semaphore.h>
#include <ace/Select_Reactor.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalSMBuffer.h"
#include "LocalSMMailbox.h"
#include "MailboxOwnerHandle.h"
#include "MessageBuffer.h"
#include "MessageFactory.h"

#include "platform/logger/Logger.h"

#include "platform/threadmgr/ThreadManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/** Singleton instance init */
LocalSMMailbox* LocalSMMailbox::localSMMailbox_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LocalSMMailbox::LocalSMMailbox(const MailboxAddress& localAddress, const char* queueName,
   const char* coordinatingMutexName) 
              : LocalMailbox (localAddress),
                localAddress_ (localAddress),
                queue_(queueName, coordinatingMutexName),
                processSemaphore_ (NULL)
{
   // Create/map the process semaphore. Initialize it to be blocked
   processSemaphore_ = new ACE_Process_Semaphore(0, localAddress.toString().c_str());

debugValue_ = true;

}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LocalSMMailbox::~LocalSMMailbox()
{
   // Flag that we are shutting down
   isShuttingDown_ = TRUE;

   // Deallocate/remove the process semaphore
   processSemaphore_->remove();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Activate the distributed mailbox
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailbox::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (isActive())
   {
      return OK;
   }//end if

   // Begin processing shared memory messages
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Local Shared Memory Mailbox activate is called",0,0,0,0,0,0);

   if (LocalMailbox::activate(mailboxOwnerHandle) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Error activating base class local mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      // Spawn a thread to start receiving shared memory messages. Here, returns OS assigned
      // unique thread Id
      ThreadManager::createThread((ACE_THR_FUNC)LocalSMMailbox::startSMProcessingThread, (void*) 0, "LocalSMMailboxProcessing", true);
   }//end else
   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Deactivate the distributed mailbox
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailbox::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Local Shared Memory Mailbox deactivate is called",0,0,0,0,0,0);

   // Base class will end the BOTH of the processing loops (Reactor and Shared Memory processing)
   return (LocalMailbox::deactivate(mailboxOwnerHandle));
}//end deactivate


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allows applications to create a mailbox and get a handle to it.
// Design:
//-----------------------------------------------------------------------------
MailboxOwnerHandle* LocalSMMailbox::createMailbox(const MailboxAddress& localAddress)
{
   // Make sure that we attempting to create the appropriate mailbox type
   if (localAddress.locationType != LOCAL_SHARED_MEMORY_MAILBOX)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Attempting to create LocalSMMailbox, but Address locationtype is %d",
         localAddress.locationType,0,0,0,0,0);
      return NULL;
   }//end if

   // Build the mailbox queue name
   ostringstream queue_ostr;
   queue_ostr << LOCALSM_QUEUENAME << "_" << localAddress.mailboxName;

   // Build the coordinating Mutex Name
   ostringstream coord_ostr;
   coord_ostr << "LocalSMCoordMutex_" << localAddress.mailboxName;

   // Allocate the singleton instance
   localSMMailbox_ = new LocalSMMailbox(localAddress, queue_ostr.str().c_str(), coord_ostr.str().c_str());
   if (!localSMMailbox_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create a local shared memory mailbox",0,0,0,0,0,0);
      return NULL;
   }//end if

   MailboxOwnerHandle* mailboxOwnerHandle = new MailboxOwnerHandle(localSMMailbox_);
   if (!mailboxOwnerHandle)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create an owner handle to local shared memory mailbox",0,0,0,0,0,0);
      return NULL;
   }//end else

   // Setup the shared memory queue (perform allocation)
   if (localSMMailbox_->queue_.setupQueue() == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to setup shared memory queue",0,0,0,0,0,0);
      return NULL;
   }//end if

   // Create a new ACE_Select_Reactor for the signal handling, etc to use
   localSMMailbox_->selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Start the Reactor Event Loop -- used for Timer processing.i
   ThreadManager::createThread((ACE_THR_FUNC)LocalMailbox::startReactor,
      (void*)localSMMailbox_->selectReactor_, "LocalMailboxReactor", true);

   return mailboxOwnerHandle;
}//end createMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string LocalSMMailbox::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& LocalSMMailbox::getMailboxAddress()
{
   return localAddress_;
}//end getMailboxAddress


//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Start the shared memory processing thread
// Design:
//-----------------------------------------------------------------------------
void LocalSMMailbox::startSMProcessingThread()
{
   // Use the singleton instance to start the loop
   localSMMailbox_->handleSMMessages();
}//end startSMProcessingThread


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handle receiving message from the shared memory queue
// Design:
//-----------------------------------------------------------------------------
void LocalSMMailbox::handleSMMessages()
{
   LocalSMBuffer sharedMemoryBuffer;
   MessageBuffer messageBuffer(MAX_MESSAGE_LENGTH, false);
   while (isActive())
   {
      // Loop performing periodic sleeps until the queue becomes non-Empty
      while (queue_.isEmpty() == true)
      {
         // sleep for 50 msec
         //usleep(50000);

         // Instead of doing a busy wait, let's do a real wait until we are signaled
         // by a post event that the queue is no longer empty. This will block as long
         // as the semaphore counter is 0; otherwise it will decrement the counter.
         // The post event will increment the counter (and thus unblock the wait)
         processSemaphore_->acquire();
      }//end while

      // NOTE: This sharedMemoryBuffer bject was copied into shared memory and here it is
      // deleted after it is dequeued. When it gets deleted, it will cause a DEVELOPER LOG
      // WARNING since we are deleting an OPMBase object.
      if (queue_.dequeueMessage(sharedMemoryBuffer) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error dequeuing message",0,0,0,0,0,0);
         // Reset the shared memory buffer
         sharedMemoryBuffer.reset();
         continue;
      }//end if   

      // Wrap the raw buffer with the Message Buffer class
      unsigned char* tempBuffer = sharedMemoryBuffer.bufferPI;
      messageBuffer.assignBuffer(tempBuffer, sharedMemoryBuffer.bufferLength); 

      // Set the insertion pointer for our Message Buffer
      messageBuffer.setInsertPosition(sharedMemoryBuffer.bufferLength);

      // Perform Message Id specific deserialization of the buffer back into a MessageBase type
      MessageBase* message = MessageFactory::recreateMessageFromBuffer(messageBuffer);
      if (message != NULL)
      {
         // First lets set the version and priority level, since these don't get serialized
         // by the developer
         message->setPriority(sharedMemoryBuffer.priorityLevel);
         //  - DO NOT DO AUTOMATIC SERIALIZATION OF VERSION...
         // BUT LEAVE THIS CODE AS EXAMPLE OF HOW TO EMBED/SERIALIZE/DESERIALIZE HIDEN/AUTOMATIC PARMS
         //message->setVersion(sharedMemoryBuffer.versionNumber);

         // Debug log
         if (debugValue_)
         {
            ostringstream debugMsg;
            char tmpBuffer[30];
            char tmpBuffer2[30];
            message->getSourceAddress().inetAddress.addr_to_string(tmpBuffer, sizeof(tmpBuffer));
            localAddress_.inetAddress.addr_to_string(tmpBuffer2, sizeof(tmpBuffer2));
            debugMsg << "##RECEIVING MESSAGE## " <<
                        " SOURCE_ADDRESS>> " << tmpBuffer << 
                        " DESTINATION_ADDRESS>> " << tmpBuffer2 << 
                        " MESSAGE_ID>> 0x" << hex << message->getMessageId() << 
                        " MESSAGE_CONTENT>> " << message->toString() << ends;
            STRACELOG(DEBUGLOG, MSGMGRLOG, debugMsg.str().c_str());
         }//end if

         incrementReceivedCount();

         // if successful, post the new message to our local mailbox
         if (post(message) == ERROR)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error enqueuing a received shared memory message to local mailbox",0,0,0,0,0,0);
         }//end if
      }//end if

      // Clear the buffer for the next loop iteration
      messageBuffer.clearBuffer();

      // Reset the shared memory buffer
      sharedMemoryBuffer.reset();
   }//end while
}//end handleSMMessages


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

