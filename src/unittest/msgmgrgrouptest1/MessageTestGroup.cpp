/******************************************************************************
*
* File name:   MessageTestGroup.cpp
* Subsystem:   Platform Services
* Description: This is a collection of unit test procedures for the MsgMgr
*              framework. This class deals exclusively with Group Mailbox
*              tests.
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

#include <unistd.h>
#include <sys/resource.h>
#include <ace/Thread_Manager.h>
#include <ace/Trace.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageTestGroup.h"
#include "platform/msgmgr/GroupMailbox.h"
#include "platform/msgmgr/MailboxProcessor.h"
#include "platform/msgmgr/MessageFactory.h"
#include "platform/msgmgr/TimerMessage.h"

#include "MessageTest1Message.h"
#include "MessageTestGroupMessage.h"
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
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)MsgMgr Group Test 1"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

#define GROUP_IP_ADDRESS "224.9.9.1"
#define GROUP_PORT_NUMBER 7777

// Static singleton instance
MessageTestGroup* MessageTestGroup::messageTestInstance_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTestGroup::MessageTestGroup()
   :messageTestGroupMailbox_(NULL),
    messageHandlerList_(NULL)
{
   // Create the Group Mailbox Address
   MailboxAddress groupAddress;
   groupAddress.locationType = GROUP_MAILBOX;
   groupAddress.mailboxName = "MessageTestGroup";
   groupAddress.inetAddress.set((short unsigned int)GROUP_PORT_NUMBER, GROUP_IP_ADDRESS); 
   groupAddress.neid = "100000001";

   // Setup the Group Mailbox
   if (!setupMailbox(groupAddress))
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to setup group mailbox",0,0,0,0,0,0);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "MessageTestGroup group mailbox setup successful",0,0,0,0,0,0);
   }//end else
   messageTestInstance_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTestGroup::~MessageTestGroup()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return static singleton instance
// Design:
//-----------------------------------------------------------------------------
MessageTestGroup* MessageTestGroup::getInstance()
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
MailboxOwnerHandle* MessageTestGroup::getOwnerHandle()
{
   return messageTestGroupMailbox_;
}//end getOwnerHandle


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup the Mailbox framework
// Design:
//-----------------------------------------------------------------------------
bool MessageTestGroup::setupMailbox(const MailboxAddress& groupAddress)
{
   // Create the Local Mailbox
   messageTestGroupMailbox_ = GroupMailbox::createMailbox(groupAddress);
   if (!messageTestGroupMailbox_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create group mailbox",0,0,0,0,0,0);
      return false;
   }//end if

   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create message handler list",0,0,0,0,0,0);
      return false;
   }//end if

   // Create handlers for each message expected
   MessageHandler testGroupMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTestGroup::processTestGroupMessage);

   MessageHandler test1MessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTestGroup::processTest1Message);

   MessageHandler timerMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTestGroup::processTimerMessage);

   // Add the message handlers to the message handler list to be used by the MsgMgr 
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor
   messageHandlerList_->add(MSGMGR_TEST_GROUP_MSG_ID, testGroupMessageHandler);

   messageHandlerList_->add(MSGMGR_TEST1_MSG_ID, test1MessageHandler);

   messageHandlerList_->add(MSGMGR_TEST_TIMER_MSG_ID, timerMessageHandler);

   // Register support for group messages so that the MessageFactory
   // knows how to recreate them in 'MessageBase' form when they are received.
   MessageBootStrapMethod testGroupMessageBootStrapMethod = makeFunctor( (MessageBootStrapMethod*)0,
                                               MessageTestGroupMessage::deserialize);
   MessageFactory::registerSupport(MSGMGR_TEST_GROUP_MSG_ID, testGroupMessageBootStrapMethod);

   return true;
}//end setupMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void MessageTestGroup::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *messageTestGroupMailbox_);
  
   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (messageTestGroupMailbox_->activate() == ERROR)
   { 
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the group side. Applications need to retry.
      TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to activate group mailbox",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Test Group Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTestGroup::processTestGroupMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   MessageTestGroupMessage* testGroupMessage = (MessageTestGroupMessage*) message;
   
   string str = "Received a Test Group Message: " + testGroupMessage->toString();
   STRACELOG(DEBUGLOG, MSGMGRLOG, str.c_str());
   return OK;
}//end processTestGroupMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Test 1 Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTestGroup::processTest1Message(MessageBase* message)
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
int MessageTestGroup::processTimerMessage(MessageBase* message)
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
string MessageTestGroup::toString()
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
void messageGroupSendingTest()
{
   // Wait to allow the processMailbox loop to initialize
   sleep(2);

   // Create the address for the MessageTestGroup Local Mailbox that we will schedule Timers on
   MailboxAddress messageTestMailboxAddress;
   // NOTE: currently, finding a Local mailbox reference in the lookup service depends on 
   // matching the locationType and mailbox name
   messageTestMailboxAddress.locationType = LOCAL_MAILBOX;
   messageTestMailboxAddress.mailboxName = "MessageTestGroup";

   // MessageTestGroup mailbox pointer 
   MailboxHandle* messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress);
   if (!messageTestMailbox)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not find Message Test Group Local Mailbox",0,0,0,0,0,0);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Successfully found Message Test Group Local Mailbox",0,0,0,0,0,0);
   }//end else

   // Create a source Mailbox Address from which the message came
   MailboxAddress sourceAddress;
   sourceAddress.mailboxName = "SendingTest";  // this mailbox doesn't exist, so just give something
   sourceAddress.locationType = GROUP_MAILBOX;

   // Create a Timer Message and schedule it. Here since we are specifying a restartInterval,
   // we do this outside of the loop to prevent exponential growth of running timers.
   ACE_Time_Value timerDelay(5);  // 5 seconds to wait
   ACE_Time_Value restartInterval(5); // restart the timer every 5 seconds - so, 10 seconds apart
   MessageTestTimerMessage* timerMessage = new MessageTestTimerMessage(sourceAddress, timerDelay, restartInterval);

   // Schedule the Timer Message with the MessageTest Local mailbox
   long timerId = MessageTestGroup::getInstance()->getOwnerHandle()->scheduleTimer(timerMessage);
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Scheduled a timer with Id %ld",timerId,0,0,0,0,0);
   
   // Go into a loop to post some messages
   while (1)
   {
      // Create a Test Group Message; create on the heap to prevent being undefined
      // when it falls out of this scope
      MessageTest1Message* test1Message = new MessageTest1Message(sourceAddress);

      // Post the Test Group Message to the MessageTestGroup Local mailbox
      // First check to see if we have a valid handle
      if (!messageTestMailbox)
      {
         messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress);
      }//end if
      // If we now have a valid handle, then post the message
      if (messageTestMailbox)
      {
         // Remember that if post is successful, then the MsgMgr framework will delete it; otherwise,
         // it is the application's responsibility to delete it or retry
         if (messageTestMailbox->post(test1Message) != ERROR)
         {
            TRACELOG(DEBUGLOG, MSGMGRLOG, "Posted Group Mailbox test message",0,0,0,0,0,0);
         }//end if
         else
         {
            delete messageTestMailbox;
            if (!(messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress)))
            {
               TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to perform MLS re-find after message post failed",0,0,0,0,0,0);
               delete test1Message;
            }//end else
            else if (messageTestMailbox->post(test1Message) == ERROR)
            {
               TRACELOG(ERRORLOG, MSGMGRLOG, "Message post failed even after MLS re-find performed",0,0,0,0,0,0);
               delete test1Message;
            }//end else if
            else
            {
               TRACELOG(WARNINGLOG, MSGMGRLOG, "Initial Group Mailbox test1 message post failed, but worked after MLS re-find",0,0,0,0,0,0);
            }//end else
         }//end else
      }//end if 

      // Pause for a bit - if this value is set to 20 usec, then it is possible
      // to witness the mailbox being overrun by posts.
      //usleep (20);
      sleep (10);
   }//end while
}//end messageGroupSendingTest


//-----------------------------------------------------------------------------
// Function Type: main function for test binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   // do some dummy stuff on bufPtr to prevent compiler warning - this code will be
   // removed by the optimizer when it runs
   int tmpInt __attribute__ ((unused)) = argc;
   char** tmpChar __attribute__ ((unused)) = argv;

   // Turn of OS limits
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // Enable ACE Tracelogs
   ACE_Trace::start_tracing();

   // Initialize the Logger with local-only output
   Logger::getInstance()->initialize(true);

   // Turn on All relevant Subsystem logging levels
   //Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
   Logger::setSubsystemLogLevel(MSGMGRLOG, DEVELOPERLOG);

   // Initialize the OPM
   OPM::initialize();

   MessageTestGroup* messageTestGroup = new MessageTestGroup();
   if (!messageTestGroup)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not create Message Test Group instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Spawn a test thread that 1.) sleeps some time to allow the mailbox processing loop
   // called next to get fully initialized, and 2.) starts sending some messages to the
   // local mailbox.
   if (ACE_Thread_Manager::instance()->spawn( (ACE_THR_FUNC) messageGroupSendingTest,
                                              (void*) 0,
                                              THR_NEW_LWP) == -1)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to spawn thread", 0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Spawning test message group sending task", 0,0,0,0,0,0);
   }//end else

   // Start the mailbox processing loop
   messageTestGroup->processMailbox();
}//end main

