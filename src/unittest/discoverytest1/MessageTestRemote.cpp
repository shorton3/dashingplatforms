/******************************************************************************
*
* File name:   MessageTestRemote.cpp
* Subsystem:   Platform Services
* Description: This is a collection of unit test procedures for the MsgMgr
*              framework. This class deals exclusively with Distributed Mailbox
*              testing of the Discovery functionality.
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
#include <unistd.h>
#include <sys/resource.h>
#include <ace/Thread_Manager.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageTestRemote.h"
#include "platform/msgmgr/DistributedMailbox.h"
#include "platform/msgmgr/MailboxLookupService.h"
#include "platform/msgmgr/MailboxProcessor.h"
#include "platform/msgmgr/MessageFactory.h"
#include "platform/msgmgr/TimerMessage.h"

#include "MessageTest1Message.h"
#include "MessageTestRemoteMessage.h"
#include "MessageTestTimerMessage.h"

#include "platform/logger/Logger.h"
#include "platform/common/MessageIds.h"

#include "platform/opm/OPM.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)DiscoveryTest1"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// Static singleton instance
MessageTestRemote* MessageTestRemote::messageTestInstance_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTestRemote::MessageTestRemote(string& localMailboxName, int localPortNumber)
   :messageTestRemoteMailbox_(NULL),
    messageHandlerList_(NULL)
{
   // Create the Distributed Mailbox Address
   distributedAddress_.locationType = DISTRIBUTED_MAILBOX;
   distributedAddress_.mailboxName = localMailboxName;
   distributedAddress_.inetAddress.set(localPortNumber, "127.0.0.1"); 
   distributedAddress_.neid = "100000001";

   // Setup the local Mailbox
   if (!setupMailbox(distributedAddress_))
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to setup distributed mailbox",0,0,0,0,0,0);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "MessageTestRemote distributed mailbox setup successful",0,0,0,0,0,0);
   }//end else
   messageTestInstance_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTestRemote::~MessageTestRemote()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return static singleton instance
// Design:
//-----------------------------------------------------------------------------
MessageTestRemote* MessageTestRemote::getInstance()
{
   if (messageTestInstance_)
   {
      return messageTestInstance_;
   }//end if
   else
   {
      return NULL;
   }//end else
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return a pointer to the owner handle for the main mailbox
// Design:
//-----------------------------------------------------------------------------
MailboxOwnerHandle* MessageTestRemote::getOwnerHandle()
{
   return messageTestRemoteMailbox_;
}//end getOwnerHandle


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return a reference to the local Mailbox Address
// Design: This is needed by the thread function and sends local messages
//-----------------------------------------------------------------------------
MailboxAddress& MessageTestRemote::getLocalMailboxAddress()
{
   return distributedAddress_;
}//end getLocalMailboxAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup the Mailbox framework
// Design:
//-----------------------------------------------------------------------------
bool MessageTestRemote::setupMailbox(const MailboxAddress& distributedAddress)
{
   // Create the Local Mailbox
   messageTestRemoteMailbox_ = DistributedMailbox::createMailbox(distributedAddress);
   if (!messageTestRemoteMailbox_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create distributed mailbox",0,0,0,0,0,0);
      return false;
   }//end if

   // Enable debug messages
   messageTestRemoteMailbox_->setDebugValue(10);

   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create message handler list",0,0,0,0,0,0);
      return false;
   }//end if

   // Create handlers for each message expected
   MessageHandler testRemoteMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTestRemote::processTestRemoteMessage);

   MessageHandler test1MessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTestRemote::processTest1Message);

   MessageHandler timerMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTestRemote::processTimerMessage);

   // Add the message handlers to the message handler list to be used by the MsgMgr 
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor
   messageHandlerList_->add(MSGMGR_TEST_DISTRIBUTED_MSG_ID, testRemoteMessageHandler);

   messageHandlerList_->add(MSGMGR_TEST1_MSG_ID, test1MessageHandler);

   messageHandlerList_->add(MSGMGR_TEST_TIMER_MSG_ID, timerMessageHandler);

   // Register support for distributed messages so that the MessageFactory
   // knows how to recreate them in 'MessageBase' form when they are received.
   MessageBootStrapMethod testRemoteMessageBootStrapMethod = makeFunctor( (MessageBootStrapMethod*)0,
                                               MessageTestRemoteMessage::deserialize);
   MessageFactory::registerSupport(MSGMGR_TEST_DISTRIBUTED_MSG_ID, testRemoteMessageBootStrapMethod);

   return true;
}//end setupMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void MessageTestRemote::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *messageTestRemoteMailbox_);
  
   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (messageTestRemoteMailbox_->activate() == ERROR)
   { 
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the remote side. Applications need to retry.
      TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to activate distributed mailbox",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Test Remote Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTestRemote::processTestRemoteMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   MessageTestRemoteMessage* testRemoteMessage = (MessageTestRemoteMessage*) message;
   
   string str = "Received a Test Remote Message: " + testRemoteMessage->toString();
   STRACELOG(DEBUGLOG, MSGMGRLOG, str.c_str());
   return OK;
}//end processTestRemoteMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Test 1 Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTestRemote::processTest1Message(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   MessageTest1Message* test1Message = (MessageTest1Message*) message;

   string str = "Received a Test1 Message: " + test1Message->toString();
   STRACELOG(DEBUGLOG, MSGMGRLOG, str.c_str());
   return OK;
}//end processTest1Messages


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Timer Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTestRemote::processTimerMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   MessageTestTimerMessage* timerMessage = (MessageTestTimerMessage*) message;

   // Do some stuff to get the expiration time in a nice format
   ACE_Time_Value timeValue = timerMessage->getExpirationTime();
   time_t time = timespec_t(timeValue).tv_sec;

   char buf[20];
   struct tm tNow;
   struct tm *t;
   t = localtime(&time);
   memcpy(&tNow, t, sizeof(struct tm));
   sprintf(buf, "%02d%02d%02d", tNow.tm_hour, tNow.tm_min, tNow.tm_sec);

   string s(buf);
   string str = "Received a Timer Message: " + s;
   STRACELOG(DEBUGLOG, MSGMGRLOG, str.c_str());

   return OK;
}//end processTimerMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MessageTestRemote::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: message sending Test
// Description:
// Design:
//-----------------------------------------------------------------------------
void messageRemoteSendingTest()
{
   // Wait to allow the processMailbox loop to initialize
   sleep(2);

   // Create the address for the MessageTestRemote Local Mailbox that we will schedule Timers on 
   MailboxAddress messageTestMailboxAddress;
   // NOTE: currently, finding a Local Mailbox mailbox reference in the lookup service depends on 
   // matching the locationType and mailbox name
   messageTestMailboxAddress.locationType = LOCAL_MAILBOX;
   messageTestMailboxAddress.mailboxName = MessageTestRemote::getInstance()->getLocalMailboxAddress().mailboxName;

   // MessageTestRemote mailbox pointer 
   MailboxHandle* messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress);
   if (!messageTestMailbox)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not find Message Test Remote Local Mailbox",0,0,0,0,0,0);
      return;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Successfully found Message Test Remote Local Mailbox",0,0,0,0,0,0);
   }//end else

   // Create a source Mailbox Address from which the message came
   MailboxAddress sourceAddress;
   sourceAddress.mailboxName = "SendingTest";  // this mailbox doesn't exist, so just give something
   sourceAddress.locationType = DISTRIBUTED_MAILBOX;
   
   // Create a Timer Message and schedule it. Here since we are specifying a restartInterval,
   // we do this outside of the loop to prevent exponential growth of running timers.
   ACE_Time_Value timerDelay(5);  // 5 seconds to wait
   ACE_Time_Value restartInterval(5); // restart the timer every 5 seconds - so, 10 seconds apart
   MessageTestTimerMessage* timerMessage = new MessageTestTimerMessage(sourceAddress, timerDelay, restartInterval);

   // Schedule the Timer Message with the MessageTest Local mailbox
   long timerId = MessageTestRemote::getInstance()->getOwnerHandle()->scheduleTimer(timerMessage);
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Scheduled a timer with Id %ld",timerId,0,0,0,0,0);

   // Go into a loop to post some messages
   while (1)
   {
      // Create a Test Remote Message; create on the heap to prevent being undefined
      // when it falls out of this scope
      MessageTest1Message* test1Message = new MessageTest1Message(sourceAddress);

      // Post the Test Remote Message to the MessageTestRemote Local mailbox
      messageTestMailbox->post(test1Message);
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Posted a Test1 message",0,0,0,0,0,0);

      // Pause for a bit - if this value is set to 20 usec, then it is possible
      // to witness the mailbox being overrun by posts.
      //usleep (20);
      sleep (10);
   }//end while
}//end messageRemoteSendingTest


//-----------------------------------------------------------------------------
// Function Type: main function for test binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   if (argc < 3)
   {
      cout << "Invalid arguments. Usage string:" << endl
           << "  DiscoveryTest1 <MailboxName> <PortNumber>" << endl;
      exit(ERROR);
   }//end if
 
   // capture the user provided local mailbox name
   string localMailboxName = argv[1];
   int localPortNumber = atoi(argv[2]);

   // Turn of OS limits
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // Initialize the Logger with local-only output
   Logger::getInstance()->initialize(true);

   // Turn on All relevant Subsystem logging levels
   Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
   Logger::setSubsystemLogLevel(MSGMGRLOG, DEVELOPERLOG);

   // Initialize the OPM
   OPM::initialize();

   // Initialize the Discovery Manager
   MailboxLookupService::initializeDiscoveryManager();

   MessageTestRemote* messageTestRemote = new MessageTestRemote(localMailboxName, localPortNumber);
   if (!messageTestRemote)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not create Message Test Remote instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Spawn a test thread that 1.) sleeps some time to allow the mailbox processing loop
   // called next to get fully initialized, and 2.) starts sending some messages to the
   // local mailbox.
   if (ACE_Thread_Manager::instance()->spawn( (ACE_THR_FUNC) messageRemoteSendingTest,
                                              (void*) 0,
                                              THR_NEW_LWP) == -1)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to spawn thread", 0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Spawning test message remote sending task", 0,0,0,0,0,0);
   }//end else

   // Start the mailbox processing loop
   messageTestRemote->processMailbox();
}//end main

