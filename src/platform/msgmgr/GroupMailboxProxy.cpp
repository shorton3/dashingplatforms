/******************************************************************************
*
* File name:   GroupMailboxProxy.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sys/socket.h>

#include <ace/Message_Block.h>
#include <ace/Select_Reactor.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "GroupMailboxProxy.h"
#include "MailboxLookupService.h"
#include "MailboxOwnerHandle.h"
#include "MessageBase.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

#include "platform/opm/OPM.h"

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
GroupMailboxProxy::GroupMailboxProxy(const MailboxAddress& groupAddress,
    unsigned int multicastLoopbackEnabled, unsigned int multicastTTL)
    :groupAddress_(groupAddress),
     multicastSocket_ (NULL),
     broadcastSocket_ (NULL),
     multicastLoopbackEnabled_ (multicastLoopbackEnabled),
     multicastTTL_ (multicastTTL),
     isMulticast_ (false)
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
GroupMailboxProxy::~GroupMailboxProxy()
{
   // Flag that we are shutting down
   isShuttingDown_ = TRUE;
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Post a message to the group remote mailbox
// Design:
//-----------------------------------------------------------------------------
int GroupMailboxProxy::post(MessageBase* messagePtr, const ACE_Time_Value* timeout)
{
   // timeout is not used here (just legacy API support for the base class), NOTE that
   // we set the reliable resend parameters in the activate method
   timeout = NULL;  // so, fix the compiler warning for unused variable

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
                  " DESTINATION_ADDRESS>> " << groupAddress_.toString() << 
                  " MESSAGE_ID>> 0x" << hex << messagePtr->getMessageId() << 
                  " MESSAGE_CONTENT>> " << messagePtr->toString() << ends;
      STRACELOG(DEBUGLOG, MSGMGRLOG, debugMsg.str().c_str());
   }//end if

   // Reserve Message Buffer object from the OPM
   MessageBuffer* messageBuffer = (MessageBuffer*)OPM_RESERVE(messageBufferPoolId_);

   // Serialize the Message Id
   *messageBuffer << messagePtr->getMessageId();

   // Serialize the remainder of the message so that the buffer can be sent to
   // the remote group mailbox
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

   // Send the message based on whether we are using multicast or broadcast
   if (isMulticast_ == true)
   {
      if (multicastSocket_->send(messageBuffer->getBuffer(), messageBuffer->getBufferLength()) <= 0)
      {
         char errorBuff[200];
         char* result = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (result == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if
         ostringstream ostr;
         ostr << "Failed to post multicast message to GroupMailboxProxy; errno (" << result << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

         // delete the message
         messagePtr->deleteMessage();

         // Release the buffer back into the OPM (and Clear the buffer) for the next post operation
         OPM_RELEASE((OPMBase*)messageBuffer);
         return ERROR;
      }//end if
   }//end if
   else if (isMulticast_ == false)
   {
      if (broadcastSocket_->send(messageBuffer->getBuffer(), messageBuffer->getBufferLength(), groupAddress_.inetAddress) <= 0)
      {
         char errorBuff[200];
         char* result = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (result == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if
         ostringstream ostr;
         ostr << "Failed to post broadcast message to GroupMailboxProxy; errno (" << result << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

         // delete the message
         messagePtr->deleteMessage();

         // Release the buffer back into the OPM (and Clear the buffer) for the next post operation
         OPM_RELEASE((OPMBase*)messageBuffer);
         return ERROR;
      }//end if
   }//end else if

   //increment the counter
   incrementSentCount();

   // delete the message
   messagePtr->deleteMessage();

   // Release the buffer back into the OPM (and Clear the buffer) for the next post operation
   OPM_RELEASE((OPMBase*)messageBuffer);

   return OK;
}//end post


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Activate the group mailbox proxy
// Design:
//-----------------------------------------------------------------------------
int GroupMailboxProxy::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (isActive())
   {
      return OK;
   }//end if

   // Begin socket IO
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Group Mailbox Proxy activate is called",0,0,0,0,0,0);

   // Determine if the IP Address specified indicates we should be using multicast
   // or broadcast
   // WE CAN'T USE THIS ONE, BROKE: isMulticast_ = groupAddress_.inetAddress.is_multicast();
   // So, we use our own from common/Defines.h instead:
   isMulticast_ = IN_MULTICAST(groupAddress_.inetAddress.get_ip_address());

   // For first-time activation, Create the Multicast socket instance. This also calls
   // the Reactor Singleton which creates the Reactor for the first time.
   if ((isMulticast_) && (multicastSocket_ == NULL))
   {
      multicastSocket_ = new ACE_SOCK_Dgram_Mcast(ACE_SOCK_Dgram_Mcast::DEFOPTS);

      // Since we will probably have other processes on this same node who are interested
      // in receiving/joining for this multicast group, we need to turn on multicast
      // loopback. If we do not, then the network stack will not send the message back
      // to its listeners -- it will only send it externally. Actually, in Linux, this
      // should be enabled by default!
      //
      // In other words, when multicast loopback is disabled, this means two applications
      // on the same machine who join the same multicast group will get each other's packets.
      // This mode has the best performance if there is always only one application joining
      // the multicast group on the machine.
      //
      // When loopback is enabled, it enables multiple applications on one machine to join
      // the same multicast group. The packets are looped back from the stack, and each
      // application is responsible for filtering out unwanted packets.
      char loop = multicastLoopbackEnabled_;     // 1 to enable loopback; 0 to disable it
      if (((ACE_SOCK*)multicastSocket_)->set_option(IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error setting multicast loopback option on socket",0,0,0,0,0,0);
      }//end if
      else if (loop == 1)
      {
         TRACELOG(DEBUGLOG, MSGMGRLOG, "Multicast Loopback enabled for group mailbox",0,0,0,0,0,0);
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, MSGMGRLOG, "Multicast Loopback disabled for group mailbox",0,0,0,0,0,0);
      }//end else

      // On multihomed hosts, if we need to get finer control on which interface the multicast
      // works on then, we need to (where addr is the IP address of the correct interface):
      // (from thread in comp.soft-sys.ace 'ACE_SOCK_Dgram_Mcast and multihomed hosts')
      if (((ACE_SOCK*)multicastSocket_)->set_option(IPPROTO_IP, IP_MULTICAST_IF,
         (char*)groupAddress_.inetAddress.get_addr(), groupAddress_.inetAddress.get_addr_size()) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error setting multicast interface option",0,0,0,0,0,0);
      }//end if

      // NOTE!! If we experience problems with Multicast packets getting from one network to another,
      // we need to probably change the TTL. By default, it is set to 1 to allow for only 1 hop
      char ttl = multicastTTL_;    // from 0 to 255
      if (((ACE_SOCK*)multicastSocket_)->set_option(IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error setting multicast TTL",0,0,0,0,0,0);
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, MSGMGRLOG, "Multicast TTL for group mailbox set to (%d)", ttl,0,0,0,0,0);
      }//end else

      // Initialize the UDP IO socket - see ace/SOCK_Dgram_Mcast.h for possible errors
      // This method performs 'join' on the datagram socket. By default, 'join' sets
      // reuse_addr to enabled!
      if ( multicastSocket_->join(groupAddress_.inetAddress) == ERROR )
      {
         ostringstream ostr;
         ostr << "Failed to open multicast datagram socket on "
              << groupAddress_.inetAddress.get_host_addr()
              << " port " << groupAddress_.inetAddress.get_port_number() << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());
         return ERROR;
      }//end if

      ostringstream ostr;
      ostr << "Opened multicast datagram socket on " << groupAddress_.inetAddress.get_host_addr()
           << " port " << groupAddress_.inetAddress.get_port_number() << ends;
      STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
   }//end if
   // For first-time activation, Create the Broadcast socket instance. This also calls
   // the Reactor Singleton which creates the Reactor for the first time.
   else if ((!isMulticast_) && (broadcastSocket_ == NULL))
   {
      // Create the broadcast socket
      broadcastSocket_ = new ACE_SOCK_Dgram_Bcast();

      // Open the socket
      if (broadcastSocket_->open(groupAddress_.inetAddress, PF_INET, 0, 1) == ERROR)
      {          TRACELOG(ERRORLOG, MSGMGRLOG, "Open broadcast socket failed",0,0,0,0,0,0);
         // IF this occurs, we need to Deactivate the LocalMailbox base class since that will de-register
         // from the MailboxLookupService
         return ERROR;
      }//end if

      ostringstream ostr;
      ostr << "Opened broadcast datagram socket on " << groupAddress_.inetAddress.get_host_addr()
           << " port " << groupAddress_.inetAddress.get_port_number() << ends;
      STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
   }//end if

   // Register the proxy mailbox with the Mailbox Lookup Service (since this doesn't inherit from LocalMailbox)
   MailboxLookupService::registerMailbox(mailboxOwnerHandle, this);

   // Set the active flag
   setActive(TRUE);
   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Deactivate the group mailbox proxy
// Design:
//-----------------------------------------------------------------------------
int GroupMailboxProxy::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (!isActive())
   {
      return OK;
   }//end if

   TRACELOG(DEBUGLOG, MSGMGRLOG, "Group mailbox proxy deactivate is called",0,0,0,0,0,0);

   // Close the datagram socket; leave the multicast group
   if ((isMulticast_ == true) && (multicastSocket_ != NULL))
   {
      if (multicastSocket_->leave(groupAddress_.inetAddress) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error leaving multicast group",0,0,0,0,0,0);
      }//end if
   }//end if
 
   setActive(FALSE);
   MailboxLookupService::deregisterMailbox(mailboxOwnerHandle);
   return OK;
}//end deactivate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the default post timeout
// Design:
//-----------------------------------------------------------------------------
const ACE_Time_Value& GroupMailboxProxy::getPostDefaultTimeout()
{
   return ACE_Time_Value::zero;
}//end getPostDefaultTimeout


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the debug flag value
// Design:
//-----------------------------------------------------------------------------
int GroupMailboxProxy::getDebugValue()
{
   return debugValue_;
}//end getDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the debug flag value
// Design:
//-----------------------------------------------------------------------------
void GroupMailboxProxy::setDebugValue(int debugValue)
{
   debugValue_ = debugValue;
}//end setDebugValue 


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allows applications to create a mailbox and get a handle to it.
// Design:
//-----------------------------------------------------------------------------
MailboxOwnerHandle* GroupMailboxProxy::createMailbox(const MailboxAddress& groupAddress,
   unsigned int multicastLoopbackEnabled, unsigned int multicastTTL)
{
   if (multicastLoopbackEnabled > 1) // unsigned, so no need to check for <0
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Invalid value for multicastLoopbackEnabled (%d)",
         multicastLoopbackEnabled,0,0,0,0,0);
      return NULL;
   }//end if
   
   if (multicastTTL > 255) // unsigned, so no need to check for <0
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Invalid value for multicastTTL (%d)",
         multicastTTL,0,0,0,0,0);
      return NULL;
   }//end if

   GroupMailboxProxy* groupMailboxProxy = new GroupMailboxProxy(groupAddress,
      multicastLoopbackEnabled, multicastTTL);
   if (!groupMailboxProxy)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create a group mailbox proxy",0,0,0,0,0,0);
      return NULL;
   }//end if

   MailboxOwnerHandle* mailboxOwnerHandle = new MailboxOwnerHandle(groupMailboxProxy);
   if (!mailboxOwnerHandle)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create an owner handle to group mailbox proxy",0,0,0,0,0,0);
   }//end else

   return mailboxOwnerHandle;
}//end createMailbox
                                                                                                                   

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string GroupMailboxProxy::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Required by base class. Not implemented
// Design:
//-----------------------------------------------------------------------------
MessageBase* GroupMailboxProxy::getMessage(unsigned short timeoutValue)
{
   timeoutValue = 0;
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call to Group Mailbox proxy getMessage",0,0,0,0,0,0);
   return NULL;
}//end getMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Required by base class. Not implemented
// Design:
//-----------------------------------------------------------------------------
MessageBase* GroupMailboxProxy::getMessageNonBlocking()
{
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call to Group Mailbox proxy getMessageNonBlocking",0,0,0,0,0,0);
   return NULL;
}//end getMessageNonBlocking


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox local address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& GroupMailboxProxy::getMailboxAddress()
{
   return groupAddress_;
}//end getMailboxAddress


//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

