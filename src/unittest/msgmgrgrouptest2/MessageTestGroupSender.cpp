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

#include "platform/msgmgr/MailboxLookupService.h"
#include "platform/msgmgr/GroupMailboxProxy.h"
#include "platform/msgmgr/MailboxHandle.h"

#include "MessageTestGroupMessage.h"

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
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)MsgMgr Group Test 2"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

#define GROUP_IP_ADDRESS "224.9.9.1"
#define GROUP_PORT_NUMBER 7777 

//-----------------------------------------------------------------------------
// Function Type: message sending Test
// Description:
// Design:
//-----------------------------------------------------------------------------
void messageGroupSender()
{
   // Create the address for the MessageTestGroup Local Mailbox that we will send to
   MailboxAddress messageTestMailboxAddress;
   // NOTE: currently, finding a mailbox reference in the lookup service depends on 
   // matching the locationType, mailbox name, IP Address and port, and neid
   messageTestMailboxAddress.locationType = GROUP_MAILBOX;
   messageTestMailboxAddress.mailboxName = "MessageTestGroup";
   messageTestMailboxAddress.inetAddress.set((short unsigned int)GROUP_PORT_NUMBER, GROUP_IP_ADDRESS);
   messageTestMailboxAddress.neid = "100000001";

   // MessageTestGroup mailbox pointer 
   MailboxHandle* messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress);
   if (!messageTestMailbox)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Could not find Message Test Group Mailbox",0,0,0,0,0,0);
      return;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Successfully found Message Test Group Mailbox",0,0,0,0,0,0);
   }//end else

   // Create a source Mailbox Address from which the message came - create some dummy values to test serialize/deserialize
   MailboxAddress sourceAddress;
   sourceAddress.locationType = GROUP_MAILBOX;
   sourceAddress.mailboxName = "TestGroupSender";  // this mailbox doesn't exist, so just give something
   sourceAddress.inetAddress.set(8888,"192.168.100.199");
   sourceAddress.slotNumber = 7;
   sourceAddress.shelfNumber = 2;
   sourceAddress.mailboxType = PHYSICAL_MAILBOX;
   sourceAddress.redundantRole = REDUNDANT_ACTIVE;
   
   // Go into a loop to post some messages
   while (1)
   {
      // Create a Test Group Message; create on the heap to prevent being undefined
      // when it falls out of this scope
      MessageTestGroupMessage* testGroupMessage = new MessageTestGroupMessage(sourceAddress, 97, "ourStringValue");

      // First test to make sure that we have a valid mailbox handle
      if (!messageTestMailbox)
      {
         messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress);
      }//end if

      // Post the Test Remote Message to the MessageTestRemote Local mailbox
      if (messageTestMailbox->post(testGroupMessage) == OK)
      {
         TRACELOG(DEBUGLOG, MSGMGRLOG, "Posted a group message",0,0,0,0,0,0);
      }//end if
      else
      {
         // Since initial posting failed, we delete the handle, get a new handle from the MLS
         // and attempt re-post
         delete messageTestMailbox;
         if (!(messageTestMailbox = MailboxLookupService::find(messageTestMailboxAddress)))
         {
            // Here, find failed so, the connection is bad and the handle ultimately needs to be deleted
            // and retried again.
            TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to perform MLS re-find after message post failed",0,0,0,0,0,0);
            delete testGroupMessage;
         }//end else
         else if (messageTestMailbox->post(testGroupMessage) == ERROR)
         {
            TRACELOG(ERRORLOG, MSGMGRLOG, "Message post failed even after MLS re-find performed",0,0,0,0,0,0);
            delete testGroupMessage;
         }//end else if
         else
         {
            TRACELOG(WARNINGLOG, MSGMGRLOG, "Initial message post failed, but worked after MLS re-find",0,0,0,0,0,0);
         }//end else
      }//end else

      // Pause for a bit - if this value is set to 20 usec, then it is possible
      // to witness the mailbox being overrun by posts.
      //usleep (20);
      sleep (3);
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

   // Loop and send group messages
   messageGroupSender();
}//end main

