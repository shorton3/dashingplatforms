/******************************************************************************
*
* File name:   TemplateApp.h
* Subsystem:   Platform Services
* Description: <Application description goes here>
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Your Name            01/01/2014 Initial release
*
*
******************************************************************************/


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <iostream>

#include <ace/Get_Opt.h>
#include <ace/Select_Reactor.h>
#include <ace/Thread_Manager.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "TemplateApp.h"

#include "platform/common/MailboxNames.h"
#include "platform/msgmgr/DistributedMailbox.h"
#include "platform/msgmgr/MailboxProcessor.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)TemplateApp"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// Some seconds to delay when shutting down
#define SHUTDOWN_DELAY 2

// Static singleton instance
TemplateApp::TemplateApp* TemplateApp::templateApp_ = NULL;

// Static Select Reactor instance
ACE_Reactor* TemplateApp::selectReactor_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
TemplateApp::TemplateApp()
               :templateAppMailbox_(NULL),
                messageHandlerList_(NULL)
{
   // Populate the static singleton instance
   templateApp_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
TemplateApp::~TemplateApp()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Catch the shutdown signal and prepare to graceful stop
// Design:
//-----------------------------------------------------------------------------
void TemplateApp::catchShutdownSignal()
{
   TRACELOG(DEBUGLOG, TEMPLATEAPPLOG, "Requested TemplateApp shutdown",0,0,0,0,0,0);
   TemplateApp::getInstance()->shutdown();
}//end catchShutdownSignal


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Retrieve the static singleton instance
// Design:
//-----------------------------------------------------------------------------
TemplateApp* TemplateApp::getInstance()
{
   if (templateApp_ == NULL)
   {
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Singleton instance is NULL",0,0,0,0,0,0);
   }//end if
   return templateApp_;
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Gracefully shutdown the Application and all of its child threads
// Design:
//-----------------------------------------------------------------------------
void TemplateApp::shutdown()
{
   // Here we need to post a message to the local mailbox to have the following
   // performed from the context of the mailbox owner thread:
   
   // Cancel the outstanding timers

   // Deactivate the Mailbox

   // Release the Mailbox for cleanup

   // Sleep for some time 
   sleep(1);

   // Shutdown the reactor
   selectReactor_->owner (ACE_Thread::self ());
   selectReactor_->end_reactor_event_loop();

   // Exit this process
   ACE::terminate_process(getpid());
}//end shutdown


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method starts the reactor thread needed for signal handling, etc.
// Design:
//-----------------------------------------------------------------------------
void TemplateApp::startReactor()
{
   // Set Reactor thread ownership
   selectReactor_->owner (ACE_Thread::self ());
   // Start the reactor processing loop
   while (selectReactor_->reactor_event_loop_done () == 0)
   {
      int result = selectReactor_->run_reactor_event_loop ();
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Reactor event loop returned with code %d",result,0,0,0,0,0);
   }//end while
}//end startReactor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialization method
// Design:
//-----------------------------------------------------------------------------
int TemplateApp::initialize(int argc, ACE_TCHAR *argv[])
{
   // Create the Distributed Mailbox Address
   MailboxAddress distributedAddress;
   distributedAddress.locationType = DISTRIBUTED_MAILBOX;
   distributedAddress.mailboxName = TEMPLATE_APP_MAILBOX_NAME;
   distributedAddress.inetAddress.set(TEMPLATE_APP_MAILBOX_PORT, LOCAL_IP_ADDRESS);

   // Setup the Distributed Mailbox
   if (!setupMailbox(distributedAddress))
   {
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Unable to setup distributed mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, TEMPLATEAPPLOG, "TemplateApp distributed mailbox setup successful",0,0,0,0,0,0);
   }//end else

   // Create a new ACE_Select_Reactor for the signal handling, etc to use
   selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create an adapter to shutdown this service and all children
   signalAdapter_ = new ACE_Sig_Adapter((ACE_Sig_Handler_Ex) TemplateApp::catchShutdownSignal);

   // Specify which types of signals we want to trap
   signalSet_.sig_add (SIGINT);
   signalSet_.sig_add (SIGQUIT);
   signalSet_.sig_add (SIGTERM);

   // Register ourselves to receive signals so we can shut down gracefully.
   if (selectReactor_->register_handler (signalSet_, signalAdapter_) == ERROR)
   {
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Error registering for OS signals",0,0,0,0,0,0);
   }//end if

   // Startup the Reactor processing loop in a dedicated thread
   if (ACE_Thread_Manager::instance()->spawn( (ACE_THR_FUNC) TemplateApp::startReactor, (void*) 0, THR_NEW_LWP) == ERROR)
   {
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Unable to spawn reactor thread", 0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, TEMPLATEAPPLOG, "Spawning TemplateApp reactor thread", 0,0,0,0,0,0);
   }//end else

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void TemplateApp::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *templateAppMailbox_);

   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (templateAppMailbox_->activate() == ERROR)
   {
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the remote side. Applications need to retry.
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Failed to activate distributed mailbox",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string TemplateApp::toString()
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
// Method Type: INSTANCE
// Description: Setup the Mailbox framework
// Design:
//-----------------------------------------------------------------------------
bool TemplateApp::setupMailbox(const MailboxAddress& distributedAddress)
{
   // Create the Local Mailbox
   templateAppMailbox_ = DistributedMailbox::createMailbox(distributedAddress);
   if (!templateAppMailbox_)
   {
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Unable to create TemplateApp mailbox",0,0,0,0,0,0);
      return false;
   }//end if

   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Unable to create message handler list",0,0,0,0,0,0);
      return false;
   }//end if

   // Create handlers for each message expected

   // Add the message handlers to the message handler list to be used by the MsgMgr
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor


   // Register support for distributed messages so that the DistributedMessageFactory
   // knows how to recreate them in 'MessageBase' form when they are received.

   return true;
}//end setupMailbox


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: main function for TemplateApp binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
   bool localLoggerMode = false;

   // Turn of OS limits and enable core file generation
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // If this application does NOT need command line arguments, then uncomment (to
   // prevent compiler warning for unused variables):
   //int tmpInt __attribute__ ((unused)) = argc;
   //char** tmpChar __attribute__ ((unused)) = argv;

   // Create usage string
   ostringstream usageString;
   string str4spaces = "    ";
   usageString << "Usage:\n  TemplateApp\n" << str4spaces 
               /* command line options */
               << "-l Local Logger Mode only (do not enqueue logs to shared memory)\n" << str4spaces 
               << "-y <argument of y> What y does...\n" << str4spaces
               << "-z What z does...\n" << str4spaces;
   usageString << ends;

   // Perform our own arguments parsing -before- we pass args to the class
   ACE_Get_Opt get_opt (argc, argv, "ly:z");
   int c;
   while ((c = get_opt ()) != ERROR)
   {
      switch (c)
      {
         case 'l':
            // Set local logger mode command line option flag (to send logs to local stdout for debugging)
            localLoggerMode = true;
            break;
         case 'y':
         {
            // Set flags and process argument of y
            char* argument_of_y = get_opt.optarg;
            //<do stuff>
            break; 
         }//end case
         case 'z':
            // Set flags for z
            break;
         default:
            cout << "Misunderstood option: " << c << endl;
            cout << usageString.str() << endl;
            exit(ERROR);
      }//end switch
   }//end while

   // Initialize the Logger to enqueue logs to the shared memory LogProcessor (unless
   // the local logger mode command line option was specified to send logs to local
   // stdout for debugging purposes)
   Logger::getInstance()->initialize(localLoggerMode);
   if (localLoggerMode)
   {
      // Turn on/off All relevant Subsystem logging levels
      //Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(TEMPLATEAPPLOG, DEVELOPERLOG);
   }//end if

   // Initialize the OPM (if we using libraries that have OPM, or if we are using our own OPM objects)
   //OPM::initialize();

   // Create the TemplateApp instance
   TemplateApp* templateApp = new TemplateApp();
   if (!templateApp)
   {
      TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Could not create TemplateApp instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Initialize the TemplateApp instance
   if (templateApp->initialize(argc, argv) == ERROR)
   {
      TRACELOG(DEBUGLOG, TEMPLATEAPPLOG, "TemplateApp failed initialization",0,0,0,0,0,0);
      exit(ERROR);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, TEMPLATEAPPLOG, "TemplateApp successfully started with PID=%d", ACE_OS::getpid(),0,0,0,0,0);
   }//end else

   // Start TemplateApp's mailbox processing loop to handle notifications. Here we block
   // and run forever until we are shutdown by the SIGINT/SIGQUIT signal
   templateApp->processMailbox();

   TRACELOG(ERRORLOG, TEMPLATEAPPLOG, "Main method exiting",0,0,0,0,0,0);
}//end main
