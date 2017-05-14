/******************************************************************************
*
* File name:   GroupMailbox.cpp
* Subsystem:   Platform Services
* Description: Mailbox class for receiving messages from both Local and
*              Group Mailbox-based Applications (based on ACE Multicast
*              Datagram protocol or Broadcast Datagram protocol).
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

#include <iostream>
using namespace std;

#include <ace/Condition_Thread_Mutex.h>
#include <ace/Message_Block.h>
#include <ace/Select_Reactor.h>
#include <ace/Thread.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "GroupMailbox.h"
#include "MailboxOwnerHandle.h"
#include "MessageFactory.h"

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
GroupMailbox::GroupMailbox(const MailboxAddress& groupAddress,
   unsigned int multicastLoopbackEnabled, unsigned int multicastTTL)
   : LocalMailbox (groupAddress), /* Base Class */
   messageBuffer_ (MAX_MESSAGE_LENGTH),
   groupAddress_ (groupAddress),
   multicastSocket_ (NULL),
   broadcastSocket_ (NULL),
   groupReactor_ (NULL),
   multicastLoopbackEnabled_ (multicastLoopbackEnabled),
   multicastTTL_ (multicastTTL),
   isMulticast_ (false)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
GroupMailbox::~GroupMailbox()
{
   // Flag that we are shutting down
   isShuttingDown_ = TRUE;

   groupReactor_->end_reactor_event_loop();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Activate the group mailbox
// Design:
//-----------------------------------------------------------------------------
int GroupMailbox::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (isActive())
   {
      return OK;
   }//end if

   // Begin listening for socket IO
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Group Mailbox activate is called",0,0,0,0,0,0);

   // Base class will start the Reactor event loop in a separate thread and
   // handle mailbox registration. We need to perform this PRIOR to opening the
   // group mailbox receiver socket, since we will need the ability to post as soon
   // as we do that.
   if (LocalMailbox::activate(mailboxOwnerHandle) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to active Local Mailbox Base for Group Mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Determine if the IP Address specified indicates we should be using multicast
   // or broadcast
   // We CAN'T USE THIS ONE, its broke:isMulticast_ = groupAddress_.inetAddress.is_multicast();
   // So, use our own in Defines.h instead:
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
         // IF this occurs, we need to Deactivate the LocalMailbox base class since
         // that will de-register from the MailboxLookupService
         LocalMailbox::deactivate(mailboxOwnerHandle);
         return ERROR;
      }//end if

      ostringstream ostr;
      ostr << "Opened multicast datagram socket on " << groupAddress_.inetAddress.get_host_addr()
           << " port " << groupAddress_.inetAddress.get_port_number() << ends;
      STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

      // Use the group reactor to demultiplex all of the messages
      if (groupReactor_->register_handler(multicastSocket_->get_handle(), this, ACE_Event_Handler::READ_MASK) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Register handler failed",0,0,0,0,0,0);
         // IF this occurs, we need to Deactivate the LocalMailbox base class since that will de-register
         // from the MailboxLookupService
         LocalMailbox::deactivate(mailboxOwnerHandle);
         return ERROR;
      }//end if
   }//end if
   // For first-time activation, Create the Broadcast socket instance. This also calls
   // the Reactor Singleton which creates the Reactor for the first time.
   else if ((!isMulticast_) && (broadcastSocket_ == NULL))
   {
      // Create the broadcast socket
      broadcastSocket_ = new ACE_SOCK_Dgram_Bcast();

      // Open the socket
      if (broadcastSocket_->open(groupAddress_.inetAddress, PF_INET, 0, 1) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Open broadcast socket failed",0,0,0,0,0,0);
         // IF this occurs, we need to Deactivate the LocalMailbox base class since that will de-register
         // from the MailboxLookupService
         LocalMailbox::deactivate(mailboxOwnerHandle);
         return ERROR;
      }//end if

      ostringstream ostr;
      ostr << "Opened broadcast datagram socket on " << groupAddress_.inetAddress.get_host_addr()
           << " port " << groupAddress_.inetAddress.get_port_number() << ends;
      STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

      // Use the group reactor to demultiplex all of the messages
      if (groupReactor_->register_handler(broadcastSocket_->get_handle(), this, ACE_Event_Handler::READ_MASK) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Register handler failed",0,0,0,0,0,0);
         // IF this occurs, we need to Deactivate the LocalMailbox base class since that will de-register
         // from the MailboxLookupService
         LocalMailbox::deactivate(mailboxOwnerHandle);
         return ERROR;
      }//end if
   }//end if

   // Force Reactor to process events
   // ACE_Time_Value tv (5, 0); // 0, 50000); // 5 seconds
   // int r = groupReactor_->handle_events (&tv);

   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Deactivate the group mailbox
// Design:
//-----------------------------------------------------------------------------
int GroupMailbox::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Group mailbox deactivate is called",0,0,0,0,0,0);

   // Close the datagram socket; leave the multicast group
   if ((isMulticast_ == true) && (multicastSocket_ != NULL))
   {
      if (multicastSocket_->leave(groupAddress_.inetAddress) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error leaving multicast group",0,0,0,0,0,0);
      }//end if
      multicastSocket_->close();
   }//end if
   else if (broadcastSocket_ != NULL)
   {
      broadcastSocket_->close();
   }//end else if

   // Base class will end the Reactor processing loop
   return (LocalMailbox::deactivate(mailboxOwnerHandle));
}//end deactivate
                                                                                                           

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Rename the group address for the mailbox
// Design:
//-----------------------------------------------------------------------------
bool GroupMailbox::rename(const MailboxAddress& newGroupAddress)
{
   // For now, do a dummy assignment to prevent the compiler from giving a warning
   const MailboxAddress tmpAddress __attribute__ ((unused)) = newGroupAddress;

   TRACELOG(ERRORLOG, MSGMGRLOG, "Incomplete implementation for rename called",0,0,0,0,0,0);
   return true;
}//end rename


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allows applications to create a mailbox and get a handle to it.
// Design:
//-----------------------------------------------------------------------------
MailboxOwnerHandle* GroupMailbox::createMailbox(const MailboxAddress& groupAddress,
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

   // Make sure that we attempting to create the appropriate mailbox type
   if (groupAddress.locationType != GROUP_MAILBOX)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Attempting to create GroupMailbox, but Address locationtype is %d",
         groupAddress.locationType,0,0,0,0,0);
      return NULL;
   }//end if

   GroupMailbox* groupMailbox = new GroupMailbox(groupAddress,
      multicastLoopbackEnabled, multicastTTL);
   if (!groupMailbox)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create a group mailbox",0,0,0,0,0,0);
      return NULL;
   }//end if

   MailboxOwnerHandle* mailboxOwnerHandle = new MailboxOwnerHandle(groupMailbox);
   if (!mailboxOwnerHandle)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create an owner handle to group mailbox",0,0,0,0,0,0);
   }//end else

   // Instantiate the ACE_Select_Reactor used by the LocalMailbox for Timers
   groupMailbox->selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create a new ACE_Select_Reactor for the signal handling, etc to use
   groupMailbox->groupReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Start the group reactor processing loop
   ThreadManager::createThread((ACE_THR_FUNC)GroupMailbox::startReactor,
      (void*) groupMailbox->groupReactor_, "GroupMailboxReactor", true);

   // Start the Reactor Event Loop -- used for Timer processing.
   ThreadManager::createThread((ACE_THR_FUNC)LocalMailbox::startReactor,
      (void*)groupMailbox->selectReactor_, "LocalMailboxReactor", true);

   return mailboxOwnerHandle;
}//end createMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description:  Overriden ACE_Event_Handler method. Called back automatically
//               when a connection has been established and upon receiving data
//               (by the Reactor framework)
// Design:
//-----------------------------------------------------------------------------
int GroupMailbox::handle_input(ACE_HANDLE handle)
{
   ACE_INET_Addr sourceUDPAddress;
   // Determine which socket the event came in on and retrieve the MessageBuffer
   if (broadcastSocket_->get_handle() == handle)
   {
      int numberBytes = 0;
      if ((numberBytes = broadcastSocket_->recv(messageBuffer_.getBuffer(), MAX_MESSAGE_LENGTH, sourceUDPAddress)) <= 0)
      {
         char errorBuff[200];
         char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (resultStr == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if
         ostringstream ostr;
         ostr << "Recv failed on group broadcast mailbox with return value (" << numberBytes
              << ") and errno (" << resultStr << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

         // Clear the buffer for the next loop iteration
         messageBuffer_.clearBuffer();
         return OK;
      }//end if

      // Set the insertion pointer for our Message Buffer
      messageBuffer_.setInsertPosition(numberBytes);
   }//end if
   else if (multicastSocket_->get_handle() == handle)
   {
      int numberBytes = 0;
      if ((numberBytes = multicastSocket_->recv(messageBuffer_.getBuffer(), MAX_MESSAGE_LENGTH, sourceUDPAddress)) <= 0)
      {
         char errorBuff[200];
         char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
         if (resultStr == NULL)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
         }//end if
         ostringstream ostr;
         ostr << "Recv failed on group multicast mailbox with return value (" << numberBytes
              << ") and errno (" << resultStr << ")" << ends;
         STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

         // Clear the buffer for the next loop iteration
         messageBuffer_.clearBuffer();
         return OK;
      }//end if

      // Set the insertion pointer for our Message Buffer
      messageBuffer_.setInsertPosition(numberBytes);
   }//end else if

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
         char tmpBuffer3[30];
         message->getSourceAddress().inetAddress.addr_to_string(tmpBuffer, sizeof(tmpBuffer));
         groupAddress_.inetAddress.addr_to_string(tmpBuffer2, sizeof(tmpBuffer2));
         sourceUDPAddress.addr_to_string(tmpBuffer3, sizeof(tmpBuffer3));
         debugMsg << "##RECEIVING MESSAGE## " <<
                     " SOURCE_ADDRESS>> " << tmpBuffer << 
                     " SOURCE_UDP_IP_ADDRESS>> " << tmpBuffer3 <<
                     " DESTINATION_ADDRESS>> " << tmpBuffer2 << 
                     " MESSAGE_ID>> 0x" << hex << message->getMessageId() << 
                     " MESSAGE_CONTENT>> " << message->toString() << ends;
         STRACELOG(DEBUGLOG, MSGMGRLOG, debugMsg.str().c_str());
      }//end if

      incrementReceivedCount();

      // if deserialization was successful, post the new message to our local mailbox 
      if (post(message) == ERROR)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error enqueuing a received group message to mailbox",0,0,0,0,0,0);
      }//end if
   }//end if

   // Clear the buffer for the next loop iteration
   messageBuffer_.clearBuffer();

   return OK;
}//end handle_input


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overriden ACE_Event_Handler method. Called back automatically
//              when a connection has been dropped
// Design:
//-----------------------------------------------------------------------------
int GroupMailbox::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
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
string GroupMailbox::toString()
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
MailboxAddress& GroupMailbox::getMailboxAddress()
{
   return groupAddress_;
}//end getMailboxGroupAddress


//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method starts the reactor thread needed for processing
//              group signaling and send/receive messages, etc.
// Design:
//-----------------------------------------------------------------------------
void GroupMailbox::startReactor(void* arg)
{
   // Convert the void* arg to a Reactor pointer
   ACE_Reactor* reactor = static_cast<ACE_Reactor*> (arg);

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
      ostr << "Group reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

      // Perform reset on the reactor
      reactor->reset_reactor_event_loop();
   }//end while

   // Need to do something to prevent the ThreadManager from restarting it
   //if (isShuttingDown_)
   //{
   //   ACE_Thread_Manager::instance()->at_exit(NULL,0);
   //}//end if
}//end startReactor


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

