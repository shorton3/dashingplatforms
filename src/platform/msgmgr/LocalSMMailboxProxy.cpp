/******************************************************************************
*
* File name:   LocalSMMailboxProxy.h
* Subsystem:   Platform Services
* Description: Proxy mailbox class for sending messages to the 'real'
*              LocalSMMailbox in another process on the same node. The message
*              exchange is performed by enqueuing the message into an
*              unbounded shared memory queue.
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

#include <ace/Process_Semaphore.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalSMMailboxProxy.h"
#include "MailboxLookupService.h"
#include "MailboxOwnerHandle.h"
#include "MessageBase.h"

#include "platform/logger/Logger.h"

#include "platform/opm/OPM.h"

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
LocalSMMailboxProxy::LocalSMMailboxProxy(const MailboxAddress& localAddress, const char* queueName,
   const char* coordinatingMutexName)
         :localAddress_(localAddress),
          queue_ (queueName, coordinatingMutexName),
          processSemaphore_ (NULL)
{
   MailboxBase::isProxy_ = true;

   // Create/map the process semaphore. Initialize it to be blocked
   processSemaphore_ = new ACE_Process_Semaphore(0, localAddress.toString().c_str());

   // Create a (Thread Safe) pool of MessageBuffer objects to be used for serialization/
   // deserialization of the Messages (use default parameters for MessageBuffer)
   MessageBufferInitializer bufferInitializer;
   bufferInitializer.bufferSize = MAX_MESSAGE_LENGTH;
   bufferInitializer.performNetworkConversion = false;
   messageBufferPoolId_ = OPM::createPool("MessageBufferDefault", (long)&bufferInitializer,
      (OPM_INIT_PTR)&MessageBuffer::initialize, 0.8, 5, 10, true, OPM_GROWTH_ALLOWED);

   // Create a Thread Safe pool of LocalSMBuffer objects for shared memory transfer
   localSMBufferPoolId_ = OPM::createPool("LocalSMBuffer", 0, (OPM_INIT_PTR)&LocalSMBuffer::initialize,
      0.8, 5, 10, true, OPM_GROWTH_ALLOWED);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LocalSMMailboxProxy::~LocalSMMailboxProxy()
{
   // Flag that we are shutting down
   isShuttingDown_ = TRUE;
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Post a message to the local shared memory mailbox
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailboxProxy::post(MessageBase* messagePtr, const ACE_Time_Value* timeout)
{
   // Prevent compiler unused variable warning
   timeout = NULL;

   if (!isActive() || (messagePtr == NULL))
   {
      return ERROR;
   }//end if

   if (debugValue_)
   {
      ostringstream debugMsg;
      char tmpBuffer[30];
      messagePtr->getSourceAddress().inetAddress.addr_to_string(tmpBuffer, sizeof(tmpBuffer));
      debugMsg << "##POSTING MESSAGE## " <<
                  " SOURCE_ADDRESS>> " << tmpBuffer << 
                  " DESTINATION_ADDRESS>> " << localAddress_.toString() << 
                  " MESSAGE_ID>> 0x" << hex << messagePtr->getMessageId() << 
                  " MESSAGECONTENT>> " << messagePtr->toString() << ends;
      STRACELOG(DEBUGLOG, MSGMGRLOG, debugMsg.str().c_str());
   }//end if

   // Reserve Message Buffer object from the OPM
   MessageBuffer* messageBuffer = (MessageBuffer*)OPM_RESERVE(messageBufferPoolId_);

   // Serialize the Message Id
   *messageBuffer << messagePtr->getMessageId();

   // Serialize the remainder of the message so that the buffer can be sent to
   // the remote distributed mailbox
   messagePtr->serialize(*messageBuffer);

   // Now, serialize the version number. We do this here for the version Number and priority level
   // so the developer doesn't have to worry about it - DO NOT DO AUTOMATIC SERIALIZATION OF VERSION...
   // BUT LEAVE THIS CODE AS EXAMPLE OF HOW TO EMBED/SERIALIZE/DESERIALIZE HIDEN/AUTOMATIC PARMS
   //*messageBuffer << messagePtr->getVersion();

   // Check to see if the Message is flagged as high priority, if so, serialize this flag to send as well
   unsigned int priorityLevel = messagePtr->getPriority();
   if (priorityLevel != 0)
   {
      *messageBuffer << priorityLevel;
   }//end if

   // Reserve a LocalSMBuffer object from the OPM
   // NOTE: This object will be copied into shared memory and later deleted after it is
   // dequeued. When it gets deleted, it will cause a DEVELOPER LOG WARNING since we are
   // deleting an OPMBase object.
   LocalSMBuffer* sharedMemoryBuffer = (LocalSMBuffer*)OPM_RESERVE(localSMBufferPoolId_);

   // Wrap the Message Buffer's raw buffer contents inside a shared memory buffer
   // Shared memory message buffer is used to encapsulate messages exchanged over shared memory
   memcpy(sharedMemoryBuffer->buffer, messageBuffer->getBuffer(), messageBuffer->getBufferLength());   
   sharedMemoryBuffer->bufferPI = sharedMemoryBuffer->buffer;
   sharedMemoryBuffer->bufferLength = messageBuffer->getBufferLength();

   if (queue_.enqueueMessage(*sharedMemoryBuffer) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to post message to Local SM Mailbox due to enqueue error. Application should must retry the message or delete it. ",0,0,0,0,0,0);
      // Release the buffer back into the OPM (and Clear the buffer) for the next post operation
      OPM_RELEASE((OPMBase*)messageBuffer);
      OPM_RELEASE((OPMBase*)sharedMemoryBuffer);
      return ERROR;
   }//end if

   // Release the Blocking Process Semaphore to wake-up the dequeue thread
   processSemaphore_->release();

   // increment the counter
   incrementSentCount();

   // delete the message (this releases to OPM if the message is poolable)
   messagePtr->deleteMessage();

   // Release the buffer back into the OPM (and Clear the buffer) for the next post operation
   OPM_RELEASE((OPMBase*)messageBuffer);
   OPM_RELEASE((OPMBase*)sharedMemoryBuffer);

   return OK;
}//end post


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Activate the local shared memory mailbox proxy
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailboxProxy::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (isActive())
   {
      return OK;
   }//end if

   TRACELOG(DEBUGLOG, MSGMGRLOG, "Local Shared Memory Mailbox Proxy activate is called",0,0,0,0,0,0);

   // Register the proxy mailbox with the Mailbox Lookup Service 
   MailboxLookupService::registerMailbox(mailboxOwnerHandle, this);

   // Set the active flag
   setActive(TRUE);
   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Deactivate the local shared memory mailbox proxy
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailboxProxy::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (!isActive())
   {
      return OK;
   }//end if

   TRACELOG(DEBUGLOG, MSGMGRLOG, "Local Shared Memory Mailbox proxy deactivate is called",0,0,0,0,0,0);

   setActive(FALSE);
   MailboxLookupService::deregisterMailbox(mailboxOwnerHandle);
   return OK;
}//end deactivate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the default post timeout
// Design:
//-----------------------------------------------------------------------------
const ACE_Time_Value& LocalSMMailboxProxy::getPostDefaultTimeout()
{
   return ACE_Time_Value::zero;
}//end getPostDefaultTimeout


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the debug flag value
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailboxProxy::getDebugValue()
{
   return debugValue_;
}//end getDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the debug flag value
// Design:
//-----------------------------------------------------------------------------
void LocalSMMailboxProxy::setDebugValue(int debugValue)
{
   debugValue_ = debugValue;
}//end setDebugValue 


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allows applications to create a mailbox and get a handle to it.
// Design:
//-----------------------------------------------------------------------------
MailboxOwnerHandle* LocalSMMailboxProxy::createMailbox(const MailboxAddress& localAddress)
{
   // Build the mailbox queue name
   ostringstream queue_ostr;
   queue_ostr << LOCALSM_QUEUENAME << "_" << localAddress.mailboxName;

   // Build the coordinating Mutex Name
   ostringstream coord_ostr;
   coord_ostr << "LocalSMCoordMutex_" << localAddress.mailboxName;

   LocalSMMailboxProxy* localSMMailboxProxy = new LocalSMMailboxProxy(localAddress, queue_ostr.str().c_str(),
      coord_ostr.str().c_str());
   if (!localSMMailboxProxy)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create a local shared memory mailbox proxy",0,0,0,0,0,0);
      return NULL;
   }//end if

   MailboxOwnerHandle* mailboxOwnerHandle = new MailboxOwnerHandle(localSMMailboxProxy);
   if (!mailboxOwnerHandle)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create an owner handle to local shared memory mailbox proxy",0,0,0,0,0,0);
      return NULL;
   }//end else

   // Setup the shared memory queue (perform allocation)
   if (localSMMailboxProxy->queue_.setupQueue() == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to setup shared memory queue",0,0,0,0,0,0);
      return NULL;
   }//end if

   return mailboxOwnerHandle;
}//end createMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string LocalSMMailboxProxy::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Required by base class. Not implemented
// Design:
//-----------------------------------------------------------------------------
MessageBase* LocalSMMailboxProxy::getMessage(unsigned short timeoutValue)
{
   timeoutValue = 0;
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call to Local Shared Memory Mailbox proxy getMessage",0,0,0,0,0,0);
   return NULL;
}//end getMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Required by base class. Not implemented
// Design:
//-----------------------------------------------------------------------------
MessageBase* LocalSMMailboxProxy::getMessageNonBlocking()
{
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call to Local Shared Memory Mailbox proxy getMessageNonBlocking",0,0,0,0,0,0);
   return NULL;
}//end getMessageNonBlocking


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& LocalSMMailboxProxy::getMailboxAddress()
{
   return localAddress_;
}//end getMailboxAddress


//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

