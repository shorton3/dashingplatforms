/******************************************************************************
*
* File name:   DistributedMailboxProxy.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DistributedMailboxProxy.h"
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
DistributedMailboxProxy::DistributedMailboxProxy(const MailboxAddress& remoteAddress)
                                                :remoteAddress_(remoteAddress)
{
   MailboxBase::isProxy_ = true;

   // Create a (Thread Safe) pool of MessageBuffer objects to be used for serialization/
   // deserialization of the Messages (use default parameters for MessageBuffer)
   MessageBufferInitializer bufferInitializer;
   bufferInitializer.bufferSize = MAX_MESSAGE_LENGTH;
   bufferInitializer.performNetworkConversion = true;
   messageBufferPoolId_ = OPM::createPool("MessageBufferDefault", (long)&bufferInitializer,
      (OPM_INIT_PTR)&MessageBuffer::initialize, 0.8, 5, 10, true, OPM_GROWTH_ALLOWED);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DistributedMailboxProxy::~DistributedMailboxProxy()
{
   // Flag that we are shutting down
   isShuttingDown_ = TRUE;
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Post a message to the distributed remote mailbox
// Design:
//-----------------------------------------------------------------------------
int DistributedMailboxProxy::post(MessageBase* messagePtr, const ACE_Time_Value* timeout)
{
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
                  " DESTINATION_ADDRESS>> " << remoteAddress_.toString() << 
                  " MESSAGE_ID>> 0x" << hex << messagePtr->getMessageId() << 
                  " MESSAGE_CONTENT>> " << messagePtr->toString() << ends;
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

   // send the buffer contents to the remote mailbox -- NOTE that send_n returns (ace/SOCK_Stream.h):
   // - On complete transfer, the number of bytes transferred is returned.
   // - On timeout, -1 is returned, errno == ETIME.
   // - On error, -1 is returned, errno is set to appropriate error.
   // - On EOF, 0 is returned, errno is irrelevant.
   if (clientStream_.send_n(messageBuffer->getBuffer(), messageBuffer->getBufferLength(), timeout) <= 0)
   {
      char errorBuff[200];
      char* result = strerror_r(errno, errorBuff, strlen(errorBuff));
      if (result == NULL)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Failed to post message to Distributed Mailbox; errno (" << result << ")" << ends;
      STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

      // First let's try to close and re-open the socket
      clientStream_.close();   
      // Attempt to re-connect to the server socket
      if ( sockConnector_.connect( clientStream_, remoteAddress_.inetAddress ) == -1 )
      {
         char errorBuff[200];
         char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (resultStr == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if

         ostringstream ostr;
         ostr << "Failed to re-connect to the distributed mailbox at " << remoteAddress_.inetAddress.get_host_addr()
              << " port " << remoteAddress_.inetAddress.get_port_number() << " with code ("
              << result << ") and errno (" << resultStr << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

         // Release the buffer back into the OPM (and Clear the buffer) for the next post operation
         OPM_RELEASE((OPMBase*)messageBuffer);
         return ERROR;
      }//end if
      // Attempt to re-post the message (we retry only once)
      if (clientStream_.send_n(messageBuffer->getBuffer(), messageBuffer->getBufferLength(), timeout) <= 0)
      {
         char errorBuff[200];
         char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (resultStr == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if

         // Prompt the user to delete the mailbox handle and re-find later (which will attempt reconnect)
         ostringstream ostr;
         ostr << "Retry post Failed with code (" << result << ") and errno (" << resultStr 
              << "). Delete proxy Mailbox Handle and re-invoke MLS::find, or delete the message" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

         // Release the buffer back into the OPM (and Clear the buffer) for the next post operation
         OPM_RELEASE((OPMBase*)messageBuffer);
         return ERROR;
      }//end if
   }//end if

   // increment the counter
   incrementSentCount();

   // delete the message (this releases to OPM if the message is poolable)
   messagePtr->deleteMessage();

   // Release the buffer back into the OPM (and Clear the buffer) for the next post operation 
   OPM_RELEASE((OPMBase*)messageBuffer);

   return OK;
}//end post


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Activate the distributed mailbox proxy
// Design:
//-----------------------------------------------------------------------------
int DistributedMailboxProxy::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (isActive())
   {
      return OK;
   }//end if

   // Begin socket IO
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Distributed Mailbox Proxy activate is called",0,0,0,0,0,0);

   // Attempt to connect to the server socket. Do this in Non-blocking mode (so that we don't block
   // the calling thread (we could also specify a timeout...). Note that we could also specify
   // the local port that gets used instead of sap_any. 1 specifies SO_REUSE_ADDR
   if ( sockConnector_.connect( clientStream_, remoteAddress_.inetAddress, 0, ACE_Addr::sap_any, 1 ) == -1 )
   {
      char errorBuff[200];
      char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
      if (resultStr == NULL)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Failed to connect to the distributed mailbox with errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());
      return ERROR;
   }//end if

   // Register the proxy mailbox with the Mailbox Lookup Service
   MailboxLookupService::registerMailbox(mailboxOwnerHandle, this);

   // Set the active flag
   setActive(TRUE); 
   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Deactivate the distributed mailbox proxy
// Design:
//-----------------------------------------------------------------------------
int DistributedMailboxProxy::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (!isActive())
   {
      return OK;
   }//end if

   TRACELOG(DEBUGLOG, MSGMGRLOG, "Distributed mailbox proxy deactivate is called",0,0,0,0,0,0);

   // Close the client socket
   clientStream_.close();
 
   setActive(FALSE); 
   MailboxLookupService::deregisterMailbox(mailboxOwnerHandle);
   return OK;
}//end deactivate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the default post timeout
// Design:
//-----------------------------------------------------------------------------
const ACE_Time_Value& DistributedMailboxProxy::getPostDefaultTimeout()
{
   return ACE_Time_Value::zero;
}//end getPostDefaultTimeout


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the debug flag value
// Design:
//-----------------------------------------------------------------------------
int DistributedMailboxProxy::getDebugValue()
{
   return debugValue_;
}//end getDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the debug flag value
// Design:
//-----------------------------------------------------------------------------
void DistributedMailboxProxy::setDebugValue(int debugValue)
{
   debugValue_ = debugValue;
}//end setDebugValue 


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allows applications to create a mailbox and get a handle to it.
// Design:
//-----------------------------------------------------------------------------
MailboxOwnerHandle* DistributedMailboxProxy::createMailbox(const MailboxAddress& remoteAddress)
{
   DistributedMailboxProxy* distributedMailboxProxy = new DistributedMailboxProxy(remoteAddress);
   if (!distributedMailboxProxy)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create a distributed mailbox proxy",0,0,0,0,0,0);
      return NULL;
   }//end if

   MailboxOwnerHandle* mailboxOwnerHandle = new MailboxOwnerHandle(distributedMailboxProxy);
   if (!mailboxOwnerHandle)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create an owner handle to distributed mailbox proxy",0,0,0,0,0,0);
   }//end else

   return mailboxOwnerHandle;
}//end createMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string DistributedMailboxProxy::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Required by base class. Not implemented
// Design:
//-----------------------------------------------------------------------------
MessageBase* DistributedMailboxProxy::getMessage(unsigned short timeoutValue)
{
   timeoutValue = 0;
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call to Distributed Mailbox proxy getMessage",0,0,0,0,0,0);
   return NULL;
}//end getMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Required by base class. Not implemented
// Design:
//-----------------------------------------------------------------------------
MessageBase* DistributedMailboxProxy::getMessageNonBlocking()
{
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call to Distributed Mailbox proxy getMessageNonBlocking",0,0,0,0,0,0);
   return NULL;
}//end getMessageNonBlocking


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox distributed address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& DistributedMailboxProxy::getMailboxAddress()
{
   return remoteAddress_;
}//end getMailboxAddress


//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

