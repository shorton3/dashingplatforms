/******************************************************************************
*
* File name:   MessageTest.cpp
* Subsystem:   Platform Services
* Description: This is a collection of unit test procedures for the MsgMgr
*              framework.
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

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageTest.h"
#include "platform/msgmgr/MailboxProcessor.h"
#include "platform/msgmgr/LocalMailbox.h"
#include "platform/msgmgr/TimerMessage.h"

#include "MessageTest1Message.h"
#include "MessageTest2Message.h"
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
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)MsgMgr Test 1"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// Static singleton instance
MessageTest* MessageTest::messageTestInstance_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTest::MessageTest()
   :messageTestMailbox_(NULL),
    messageHandlerList_(NULL)
{
   // Create the LocalMailbox Address
   MailboxAddress localAddress;
   localAddress.locationType = LOCAL_MAILBOX;
   localAddress.mailboxName = "MessageTest";

   // Setup the local Mailbox
   if (!setupMailbox(localAddress))
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to setup local mailbox",0,0,0,0,0,0);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "MessageTest local mailbox setup successful",0,0,0,0,0,0);
   }//end else
   messageTestInstance_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageTest::~MessageTest()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return static singleton instance
// Design:
//-----------------------------------------------------------------------------
MessageTest* MessageTest::getInstance()
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
MailboxOwnerHandle* MessageTest::getOwnerHandle()
{
   return messageTestMailbox_;
}//end getOwnerHandle


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup the Mailbox framework
// Design:
//-----------------------------------------------------------------------------
bool MessageTest::setupMailbox(const MailboxAddress& address)
{
   // Create the Local Mailbox
   messageTestMailbox_ = LocalMailbox::createMailbox(address);
   if (!messageTestMailbox_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create local mailbox",0,0,0,0,0,0);
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
   MessageHandler test1MessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTest::processTest1Message);

   MessageHandler test2MessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTest::processTest2Message);

   MessageHandler timerMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &MessageTest::processTimerMessage);

   // Add the message handlers to the message handler list to be used by the MsgMgr 
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor
   messageHandlerList_->add(MSGMGR_TEST1_MSG_ID, test1MessageHandler);

   messageHandlerList_->add(MSGMGR_TEST2_MSG_ID, test2MessageHandler);

   messageHandlerList_->add(MSGMGR_TEST_TIMER_MSG_ID, timerMessageHandler);
   return true;
}//end setupMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void MessageTest::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *messageTestMailbox_);
  
   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (messageTestMailbox_->activate() == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Local Mailbox activation failed",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox



//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Test 1 Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTest::processTest1Message(MessageBase* message)
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
// Description: Handler to Process Test 2 Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTest::processTest2Message(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   MessageTest2Message* test2Message = (MessageTest2Message*) message;

   string str = "Received a Test2 Message: " + test2Message->toString();
   STRACELOG(DEBUGLOG, MSGMGRLOG, str.c_str());
   return OK;
}//end processTest2Messages


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Timer Messages
// Design:
//-----------------------------------------------------------------------------
int MessageTest::processTimerMessage(MessageBase* message)
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
string MessageTest::toString()
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
void messageSendingTest()
{
   // Wait to allow the processMailbox loop to initialize
   sleep(2);

   // Create the address for the MessageTest Local Mailbox that we will send to
   MailboxAddress messageTestMailboxAddress;
   messageTestMailboxAddress.locationType = LOCAL_MAILBOX;
   messageTestMailboxAddress.mailboxName = "MessageTest";

   // MessageTest mailbox pointer 
   MailboxHandle* messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress);
   if (!messageTestMailbox)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not find Message Test Local Mailbox",0,0,0,0,0,0);
      return;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Successfully found Message Test Local Mailbox",0,0,0,0,0,0);
   }//end else

   // Create a source Mailbox Address from which the message came
   MailboxAddress sourceAddress;
   sourceAddress.mailboxName = "SendingTest";  // this mailbox doesn't exist, so just give something

   // Create a Timer Message and schedule it. Here since we are specifying a restartInterval,
   // we do this outside of the loop to prevent exponential growth of running timers.
   ACE_Time_Value timerDelay(5);  // 5 seconds to wait
   ACE_Time_Value restartInterval(5); // restart the timer every 5 seconds - so, 10 seconds apart
   MessageTestTimerMessage* timerMessage = new MessageTestTimerMessage(sourceAddress, timerDelay, restartInterval);

   // Schedule the Timer Message with the MessageTest Local mailbox
   long timerId = MessageTest::getInstance()->getOwnerHandle()->scheduleTimer(timerMessage);
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Scheduled a timer with Id %ld",timerId,0,0,0,0,0);
   
   // Go into a loop to post some messages
   while (1)
   {
      // Create a Test1 Message; create on the heap to prevent being undefined
      // when it falls out of this scope
      MessageTest1Message* test1Message = new MessageTest1Message(sourceAddress);

      // Post the Test1 Message to the MessageTest Local mailbox
      messageTestMailbox->post(test1Message);

      // Create a Test2 Message; create on the heap to prevent being undefined
      // when it falls out of this scope
      MessageTest2Message* test2Message = new MessageTest2Message(sourceAddress);

      // Post the Test2 Message to the MessageTest Local mailbox
      messageTestMailbox->post(test2Message);

      // Pause for a bit - if this value is set to 20 usec, then it is possible
      // to witness the mailbox being overrun by posts.
      //usleep (20);
      sleep (30);
   }//end while
}//end messageSendingTest


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

   // Initialize the Logger with local-only output
   Logger::getInstance()->initialize(true);

   // Turn on All relevant Subsystem logging levels
   //Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
   Logger::setSubsystemLogLevel(MSGMGRLOG, DEVELOPERLOG);

   // Initialize the OPM
   OPM::initialize();

   MessageTest* messageTest = new MessageTest();
   if (!messageTest)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not create Message Test instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Spawn a test thread that 1.) sleeps some time to allow the mailbox processing loop
   // called next to get fully initialized, and 2.) starts sending some messages to the
   // local mailbox.
   if (ACE_Thread_Manager::instance()->spawn( (ACE_THR_FUNC) messageSendingTest,
                                              (void*) 0,
                                              THR_NEW_LWP) == -1)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to spawn thread", 0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Spawning test message sending task", 0,0,0,0,0,0);
   }//end else

   // Start the mailbox processing loop
   messageTest->processMailbox();
}//end main

