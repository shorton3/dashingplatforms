/******************************************************************************
*
* File name:   ProcessManager.cpp
* Subsystem:   Platform Services
* Description: The process manager provides for the configuration and startup
*              of all platform and application processes and process monitoring
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

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>

#include <ace/Get_Opt.h>
#include <ace/Process_Manager.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Reactor.h>
#include <ace/Select_Reactor.h>
#include <ace/Service_Config.h>
#include <ace/Sig_Adapter.h>
#include <ace/Thread_Manager.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ProcessManager.h"

#include "platform/common/MailboxNames.h"
#include "platform/msgmgr/LocalMailbox.h"
#include "platform/msgmgr/MailboxProcessor.h"

#include "platform/logger/Logger.h"

#include "platform/opm/OPM.h"

#include "platform/threadmgr/ThreadManager.h"

#include "platform/utilities/SystemInfo.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Process Manager"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

#define SHUTDOWN_DELAY 4

// Select Reactor used by Process Manager and signal handling
ACE_Reactor* ProcessManager::selectReactor_ = NULL;

// Static Singleton instance
ProcessManager::ProcessManager* ProcessManager::ourProcessManager_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ProcessManager::ProcessManager()
               :processManagerMailbox_(NULL),
                messageHandlerList_(NULL),
                processManager_(NULL)
{
   // Populate the static singleton instance
   ourProcessManager_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ProcessManager::~ProcessManager()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Retrieve the static singleton instance
// Design:
//-----------------------------------------------------------------------------
ProcessManager* ProcessManager::getInstance()
{
   if (ourProcessManager_ == NULL)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Singleton instance is NULL",0,0,0,0,0,0);
   }//end if
   return ourProcessManager_;
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Catch the shutdown signal and prepare to graceful stop
// Design:
//-----------------------------------------------------------------------------
void ProcessManager::catchShutdownSignal()
{
   TRACELOG(DEBUGLOG, PROCMGRLOG, "Requested Process Manager shutdown",0,0,0,0,0,0);
   ProcessManager::getInstance()->shutdown();
}//end catchShutdownSignal


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Gracefully shutdown the Process Manager and all of its child processes
// Design:
//-----------------------------------------------------------------------------
void ProcessManager::shutdown()
{
   TRACELOG(DEBUGLOG, PROCMGRLOG, "Handling Process Manager shutdown",0,0,0,0,0,0);

   // Shutdown the service configurator. This will call 'fini' which will instruct
   // the ACE_Process_Manager to terminate the processes 
   ACE_Service_Config::close();

   // Instruct Process Manager to wait (indefinitely) for all child processes to exit
   if (processManager_->wait() == ERROR)  // Wait for ACE Max Time
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Error with process manager waiting on child processes to exit",0,0,0,0,0,0);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, PROCMGRLOG, "Process Manager finished waiting for child processes to exit. Now self exitting.",0,0,0,0,0,0);
   }//end else

   // Sleep sometime to allow the Service Configurator to close the child processes
   //ACE_OS::sleep(SHUTDOWN_DELAY);

   // Shutdown the process manager
   //ACE_Process_Manager::instance ()->close ();
                                                                                                                                              
   // Shutdown the reactor
   //selectReactor_->owner (ACE_Thread::self ());
   //selectReactor_->end_reactor_event_loop();

   // Exit this process
   ACE::terminate_process(getpid());
}//end shutdown


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method starts the reactor thread needed for processing
//              Process Death Handlers and auto-reaping the exit statuses of the
//              child processes
// Design:
//-----------------------------------------------------------------------------
void ProcessManager::startReactor()
{
   // Set Reactor thread ownership
   selectReactor_->owner (ACE_Thread::self ());
   // Start the reactor processing loop
   while (selectReactor_->reactor_event_loop_done () == 0)
   {
      int result = selectReactor_->run_reactor_event_loop ();

      char errorBuff[200];
      char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
      if (resultStr == NULL)
      {
         TRACELOG(ERRORLOG, PROCMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());
   }//end while
}//end startReactor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialization method
// Design:
//-----------------------------------------------------------------------------
int ProcessManager::initialize(int argc, ACE_TCHAR *argv[], string usageString)
{
   if (strcmp(SystemInfo::getLocalNEID().c_str(), "000000000") == 0)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Invalid NEID detected, exiting",0,0,0,0,0,0);
      return ERROR;
   }//end if

   ostringstream ostr;
   ostr << "Process Manager initialization started for NEID ("
        << SystemInfo::getLocalNEID() << ") on Shelf ("
        << SystemInfo::getShelfNumber() << ") on Slot ("
        << SystemInfo::getSlotNumber() << ")" << ends;
   STRACELOG(DEBUGLOG, PROCMGRLOG, ostr.str().c_str());

   // Create the Local Mailbox Address
   MailboxAddress localAddress;
   localAddress.locationType = LOCAL_MAILBOX;
   localAddress.mailboxName = PROCMGR_MAILBOX_NAME;

   // Setup the local Mailbox
   if (!setupMailbox(localAddress))
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Unable to setup local mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, PROCMGRLOG, "Process Manager local mailbox setup successful",0,0,0,0,0,0);
   }//end else

   // Create a new ACE_Select_Reactor for the signal handling, process manager, etc to use
   selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create an adapter to shutdown this service and all child applications
   signalAdapter_ = new ACE_Sig_Adapter((ACE_Sig_Handler_Ex) ProcessManager::catchShutdownSignal);

   // Specify which types of signals we want to trap
   signalSet_.sig_add (SIGINT);
   signalSet_.sig_add (SIGQUIT);
   signalSet_.sig_add (SIGTERM);

   // Register ourselves to receive signals so we can shut down gracefully.
   if (selectReactor_->register_handler (signalSet_, signalAdapter_) == ERROR)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Error registering for OS signals",0,0,0,0,0,0);
   }//end if 

   // Initialize the ACE Process Manager with a default size and pass in the 
   // ACE Reactor. Process_Manager default size is 100...this should be adjusted at
   // some time later.
   processManager_ = new ACE_Process_Manager((size_t)ACE_Process_Manager::DEFAULT_SIZE, selectReactor_);

   // Set the pointer to the process wide ACE_Process_Manager (will be used within ProcessController)
   ACE_Process_Manager::instance(processManager_);

   // Run the ACE Service Configurator to read and parse the configuration
   if (ACE_Service_Config::open (argc, argv, ACE_DEFAULT_LOGGER_KEY, 0) == ERROR)
   {
      STRACELOG(ERRORLOG, PROCMGRLOG, usageString.c_str());
      return ERROR;
   }//end if

   // Startup the Reactor processing loop in a dedicated thread. Here, returns the OS assigned
   // unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)ProcessManager::startReactor, (void*) 0, "ProcessManagerReactor", true);

   size_t numberProcs = processManager_->managed ();
   TRACELOG(DEBUGLOG, PROCMGRLOG, "Process Manager currently managing %d child processes",numberProcs,0,0,0,0,0);

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void ProcessManager::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *processManagerMailbox_);

   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (processManagerMailbox_->activate() == ERROR)
   {
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the remote side. Applications need to retry.
      TRACELOG(ERRORLOG, PROCMGRLOG, "Failed to activate local mailbox",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string ProcessManager::toString()
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
bool ProcessManager::setupMailbox(const MailboxAddress& localAddress)
{
   // Create the Local Mailbox
   processManagerMailbox_ = LocalMailbox::createMailbox(localAddress);
   if (!processManagerMailbox_)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Unable to create local mailbox",0,0,0,0,0,0);
      return false;
   }//end if
                                                                                                                        
   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Unable to create message handler list",0,0,0,0,0,0);
      return false;
   }//end if

   // Create handlers for each message expected

   // Add the message handlers to the message handler list to be used by the MsgMgr
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor


   return true;
}//end setupMailbox


/** NOTE: When we received mailbox messages regarding the processes (based on
    processID), then iterate through the Service Config. To do this, we need to
    have a list of ProcessControllers and their PIDs or their Names, but since
    Service_Config created them, this is hard to do. Either we need to create a
    map of ProcessControllers ourselves -or- we need to use the Service Repository
    to do it. ??
*/


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: main function for process manager binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
   char* redirectFile = NULL;
   bool localLoggerMode = false;

   // Use ACE to daemonize the ProcessManager - Here we specify the root directory for
   // the new process ("/"), and we specify 0 to indicate that open file handles should
   // NOT be closed (this will prevent us from being able to redirect stdout/stderr logs).
   ACE::daemonize(ACE_TEXT ("/"), 0);

   // Set file creation mask to 0664 (ACE_DEFAULT_FILE_PERMS - is 644)
   ACE_OS::umask(002);

   // Since daemonizing sets the current directory to /, execute a chroot
   // back to the $DEV_ROOT directory
   char tmpBuff[50];
   sprintf(tmpBuff, "%s/bin/", getenv("DEV_ROOT"));
   if (chdir(tmpBuff) == ERROR)
   {
      perror("Process Manager : Error setting current directory to DEV_ROOT");
   }//end if

   // Turn of OS limits and enable core file generation
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // Create usage string
   ostringstream usageString;
   string str4spaces = "    ";
   usageString << "Usage:\n  ProcessManager\n" << str4spaces 
               /* Service Config specific options */
               << "-b Daemonize the Process Manager process\n" << str4spaces 
               << "-d Verbose diagnostics as process directives are forked\n" << str4spaces
               << "-f Read a different file other than svc.conf\n" << str4spaces
               << "-n Do not process static directives\n" << str4spaces
               << "-s Designate which signal to use for reconfigure instead of SIGHUP\n" << str4spaces
               << "-S Pass in a process directive not specified in the svc.conf file\n" << str4spaces
               << "-y Process static directives\n" << str4spaces
               /* Process Manager/Platform specific options */
               << "-l Local Logger Mode only (do not enqueue logs to shared memory)\n" << str4spaces
               << "-r Redirect stdout/stderr to a specified file\n" << str4spaces;
   usageString << ends;

   // Perform our own arguments parsing -before- we pass args to the Service Configurator
   ACE_Get_Opt get_opt (argc, argv, "lbdf:ns:S:yvr:");
   int c;
   while ((c = get_opt ()) != ERROR)
   {
      switch (c)
      {
         case 'l':
            // Set flag for Process Manager to use Local Logger Mode
            localLoggerMode = true;
            break;
         case 'r':
         {
            // Redirect stdout and stderr to the following file
            redirectFile = get_opt.optarg;
            ACE_OS::unlink (redirectFile);
            // Use freopen to redirect; however, we could use stdio dup2 call to copy
            // ACE_STDOUT/ACE_STDERR
            freopen (redirectFile, "a+", stdout);
            freopen (redirectFile, "a+", stderr);
            cout << "Output redirected to " << redirectFile << endl << flush;
            // Redirect the ACE Logger's output to our text file. This affects ACE components that
            // use the Logger by default (such as the Service Configurator). NOTE!! If we want to
            // redirect the Platform Logger to the OS Syslog facility, then we will need to re-do
            // this configuration at that time (see ace/Log_Msg.h for flag definitions)
            ofstream out (redirectFile, fstream::in | fstream::out | fstream::app);
            ACE_Log_Msg::instance()->msg_ostream(&out);
            ACE_Log_Msg::instance()->set_flags(ACE_Log_Msg::OSTREAM);
            ACE_Log_Msg::instance()->clr_flags(ACE_Log_Msg::STDERR | ACE_Log_Msg::LOGGER);
            break; 
         }//end case
         case 'b':
            break;
         case 'd':
            break;
         case 'f':
            break;
         case 'n':
            break;
         case 's':
            break;
         case 'S':
            break;
         case 'y':
            break;
         default:
            cout << "Misunderstood option: " << c << endl;
            cout << usageString.str() << endl;
            exit(ERROR);
      }//end switch
   }//end while

   // Initialize the Logger; Set the logger to enqueue Log Messages to the shared
   // memory queue for the Log Processor to dequeue them (unless the localLoggerMode
   // command line parameter option was specified)
   Logger::getInstance()->initialize(localLoggerMode);
   if (localLoggerMode)
   {
      // Turn on All relevant Subsystem logging levels
      //Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(PROCMGRLOG, DEVELOPERLOG);
   }//end if

   // Initialize the OPM
   OPM::initialize();

   // Create the process manager instance
   ProcessManager* processManager = new ProcessManager();
   if (!processManager)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Could not create Process Manager instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Initialize the process manager instance
   if (processManager->initialize(argc, argv, usageString.str()) == ERROR)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Process Manager failed initialization",0,0,0,0,0,0);
      exit(ERROR);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, PROCMGRLOG, "Process Manager successfully started with PID=%d", ACE_OS::getpid(),0,0,0,0,0);
   }//end else

   // Start process manager's mailbox processing loop to handle notifications. Here we block
   // and run forever until we are shutdown by the SIGINT/SIGQUIT signal
   processManager->processMailbox();

   TRACELOG(ERRORLOG, PROCMGRLOG, "Main method exiting",0,0,0,0,0,0);
}//end main
