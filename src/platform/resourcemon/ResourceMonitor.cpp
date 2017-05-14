/******************************************************************************
*
* File name:   ResourceMonitor.h
* Subsystem:   Platform Services
* Description: Resource Monitor provides auditing of critical card resources,
*              for example disk usage, memory usage, and cpu usage.
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

#include <ace/Get_Opt.h>
#include <ace/Select_Reactor.h>
#include <ace/Sig_Adapter.h>
#include <ace/Thread_Manager.h>
#include <ace/Time_Value.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ResourceMonitor.h"
#include "ResourceMonitorTimerMessage.h"

#include "platform/datamgr/DataManager.h"
#include "platform/datamgr/DbConnectionHandle.h"
#include "platform/common/ConnectionSetNames.h"

#include "platform/common/MailboxNames.h"
#include "platform/common/MessageIds.h"
#include "platform/msgmgr/DistributedMailbox.h"
#include "platform/msgmgr/MailboxProcessor.h"

#include "platform/logger/Logger.h"

#include "platform/faultmgr/Faults.h"

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
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Resource Monitor"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// Monitoring interval for checking OS Resources (in seconds)
#define OS_RESOURCE_MONITORING_INTERVAL 45

// Static singleton instance
ResourceMonitor::ResourceMonitor* ResourceMonitor::resourceMonitor_ = NULL;

// Static Select Reactor instance
ACE_Reactor* ResourceMonitor::selectReactor_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ResourceMonitor::ResourceMonitor()
               :resourceMonitorMailbox_(NULL),
                messageHandlerList_(NULL),
                monitoringTimerId_(0),
                monitoringTimerMessage_(NULL)
{
   // Populate the static singleton instance
   resourceMonitor_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ResourceMonitor::~ResourceMonitor()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Catch the shutdown signal and prepare to graceful stop
// Design:
//-----------------------------------------------------------------------------
void ResourceMonitor::catchShutdownSignal()
{
   TRACELOG(DEBUGLOG, RSRCMONLOG, "Requested Resource Monitor shutdown",0,0,0,0,0,0);
   ResourceMonitor::getInstance()->shutdown();
   //delete ResourceMonitor::getInstance();
}//end catchShutdownSignal


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Retrieve the static singleton instance
// Design:
//-----------------------------------------------------------------------------
ResourceMonitor* ResourceMonitor::getInstance()
{
   if (resourceMonitor_ == NULL)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Singleton instance is NULL",0,0,0,0,0,0);
   }//end if
   return resourceMonitor_;
}//end getInstance
   

//-----------------------------------------------------------------------------
// Method Type: PRIVATE
// Description: Gracefully shutdown the Application and all of its child threads
// Design:
//-----------------------------------------------------------------------------
void ResourceMonitor::shutdown()
{
   // Use static flag to test if shutdown has been called multiple times. If this
   // is a subsequent call, this means shutdown was not successful the first time,
   // so now, let's just DIE!!
   static bool shutdownPreviouslyCalled = false;
   if (shutdownPreviouslyCalled == false)
   {
      shutdownPreviouslyCalled = true;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, RSRCMONLOG, "Resource Monitor requested subsequent shutdown, exiting...",0,0,0,0,0,0);
      ACE::terminate_process(getpid());
   }//end else
 
   // Here, we need to post a message to the ResourceMonitor mailbox queue to have the
   // ResourceMonitor's mailbox deactivated and released from within its owner thread
   //
   // Cancel the outstanding timers
   //if (resourceMonitorMailbox_->cancelTimer(monitoringTimerId_, monitoringTimerMessage_) == ERROR)
   //{
   //   TRACELOG(ERRORLOG, RSRCMONLOG, "Error canceling Resource Monitoring Timer for shutdown",0,0,0,0,0,0);
   //}//end if
   //
   // Deactivate the Mailbox
   //if (resourceMonitorMailbox_->deactivate() == ERROR)
   //{
   //   TRACELOG(ERRORLOG, RSRCMONLOG, "Error deactivating Resource Monitoring Mailbox for shutdown",0,0,0,0,0,0);
   //}//end if
   //
   // Release the Mailbox for cleanup
   //resourceMonitorMailbox_->release();

   // Shutdown the reactor
   selectReactor_->owner (ACE_Thread::self ());
   selectReactor_->end_reactor_event_loop();

   ACE::terminate_process(getpid());
}//end shutdown


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method starts the reactor thread needed for signal handling, etc.
// Design:
//-----------------------------------------------------------------------------
void ResourceMonitor::startReactor()
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
         TRACELOG(ERRORLOG, RSRCMONLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, RSRCMONLOG, ostr.str().c_str());
   }//end while
}//end startReactor

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialization method
// Design:
//-----------------------------------------------------------------------------
int ResourceMonitor::initialize(int argc, ACE_TCHAR *argv[])
{
   // For now, to prevent the compiler warning:
   int tmpArgc __attribute__ ((unused)) = argc;
   char** tmpChar __attribute__ ((unused)) = argv;

   // Initialize the DataManager and activate database connections first!
   if (DataManager::initialize("./DataManager.conf") == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error initializing Data manager",0,0,0,0,0,0);
      // To allow for development, do not return
      // return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, RSRCMONLOG, "Successfully initialized Data Manager",0,0,0,0,0,0);
   }//end else

   // Activate the Logger Connection Set
   if (DataManager::activateConnectionSet(RESOURCE_MONITOR_CONNECTION) == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error activating Resource Monitor ConnectionSet",0,0,0,0,0,0);
      // To allow for development, do not return
      // return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, RSRCMONLOG, "Successfully activated Resource Monitor ConnectionSet",0,0,0,0,0,0);
   }//end else

   // Create the Distributed Mailbox Address
   MailboxAddress distributedAddress;
   distributedAddress.locationType = DISTRIBUTED_MAILBOX;
   distributedAddress.mailboxName = RESOURCE_MONITOR_MAILBOX_NAME;
   distributedAddress.inetAddress.set(RESOURCE_MONITOR_MAILBOX_PORT, LOCAL_IP_ADDRESS);
   distributedAddress.neid = SystemInfo::getLocalNEID();

   // Setup the distributed Mailbox
   if (!setupMailbox(distributedAddress))
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Unable to setup distributed mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, RSRCMONLOG, "ResourceMonitor distributed mailbox setup successful",0,0,0,0,0,0);
   }//end else

   // Create a new ACE_Select_Reactor for the signal handling, etc to use
   selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create an adapter to shutdown this service and all children
   signalAdapter_ = new ACE_Sig_Adapter((ACE_Sig_Handler_Ex) ResourceMonitor::catchShutdownSignal);

   // Specify which types of signals we want to trap
   signalSet_.sig_add (SIGINT);
   signalSet_.sig_add (SIGQUIT);
   signalSet_.sig_add (SIGTERM);

   // Register ourselves to receive signals so we can shut down gracefully.
   if (selectReactor_->register_handler (signalSet_, signalAdapter_) == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error registering for OS signals",0,0,0,0,0,0);
   }//end if

   // Startup the Reactor processing loop in a dedicated thread. Here, returns the OS assigned
   // unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)ResourceMonitor::startReactor, (void*) 0, "ResourceMonitorReactor", true);

   // Initialize the OS Resource Monitoring Module
   if (osResourceMonitor_.initialize() == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "OS Resource Monitor failed to initialize",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Setup the PERIODIC Timer for checking resources
   ACE_Time_Value timerDelay(OS_RESOURCE_MONITORING_INTERVAL);  // seconds to wait
   ACE_Time_Value restartInterval(OS_RESOURCE_MONITORING_INTERVAL); // restart the timer every x seconds
   monitoringTimerMessage_ = new ResourceMonitorTimerMessage(distributedAddress, timerDelay, restartInterval);

   // Schedule the Timer Message with the Resource Monitor Local mailbox
   monitoringTimerId_ = resourceMonitorMailbox_->scheduleTimer(monitoringTimerMessage_);
   TRACELOG(DEBUGLOG, RSRCMONLOG, "Scheduled a timer with Id %ld for periodic resource usage collection",monitoringTimerId_,0,0,0,0,0);

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void ResourceMonitor::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *resourceMonitorMailbox_);

   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (resourceMonitorMailbox_->activate() == ERROR)
   {
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the remote side. Applications need to retry.
      TRACELOG(ERRORLOG, RSRCMONLOG, "Failed to activate distributed mailbox",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string ResourceMonitor::toString()
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
// Description: Process Timer Message to perform measurements and checks
// Design:
//-----------------------------------------------------------------------------
int ResourceMonitor::processTimerMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   ResourceMonitorTimerMessage* timerMessage = (ResourceMonitorTimerMessage*) message;

   // DUMMY CODE: For now, we don't need timer message, so to prevent the compiler warning:
   timerMessage = NULL;

   // Here, we do periodic checking of resources
   if (osResourceMonitor_.checkDiskUsage() == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error checking disk usage statistics",0,0,0,0,0,0);
      return ERROR;
   }//end if
   if (osResourceMonitor_.checkCPUUsage() == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error checking CPU usage statistics",0,0,0,0,0,0);
      return ERROR;
   }//end if
   if (osResourceMonitor_.checkMemoryUsage() == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error checking Memory usage statistics",0,0,0,0,0,0);
      return ERROR;
   }//end if
   
   return OK;
}//end processTimerMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup the Mailbox framework
// Design:
//-----------------------------------------------------------------------------
bool ResourceMonitor::setupMailbox(const MailboxAddress& distributedAddress)
{
   // Create the Local Mailbox
   resourceMonitorMailbox_ = DistributedMailbox::createMailbox(distributedAddress);
   if (!resourceMonitorMailbox_)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Unable to create ResourceMonitor mailbox",0,0,0,0,0,0);
      return false;
   }//end if

   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Unable to create message handler list",0,0,0,0,0,0);
      return false;
   }//end if

   // Create handlers for each message expected
   MessageHandler timerMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &ResourceMonitor::processTimerMessage);

   // Add the message handlers to the message handler list to be used by the MsgMgr
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor
   messageHandlerList_->add(RESOURCE_MONITOR_TIMER_MSG_ID, timerMessageHandler);

   // Register support for distributed messages so that the DistributedMessageFactory
   // knows how to recreate them in 'MessageBase' form when they are received.


   return true;
}//end setupMailbox


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: main function for ResourceMonitor binary
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
   int tmpInt __attribute__ ((unused)) = argc;
   char** tmpChar __attribute__ ((unused)) = argv;

   // Create usage string
   ostringstream usageString;
   string str4spaces = "    ";
   // Command line options:
   usageString << "Usage:\n  ResourceMonitor\n" << str4spaces 
               << "-l Local Logger Mode only (do not enqueue logs to shared memory)\n" << str4spaces; 
   usageString << ends;

   // Perform our own arguments parsing -before- we pass args to the Service Configurator
   ACE_Get_Opt get_opt (argc, argv, "l");
   int c;
   while ((c = get_opt ()) != ERROR)
   {
      switch (c)
      {
         case 'l':
         {
            // Set local logger mode option flag
            localLoggerMode = true;
            break; 
         }//end case
         default:
            cout << "Misunderstood option: " << c << endl;
            cout << usageString.str() << endl;
            exit(ERROR);
      }//end switch
   }//end while

   // Initialize the Logger with output sent to the shared memory queue for Log Processor
   Logger::getInstance()->initialize(localLoggerMode);
   if (localLoggerMode)
   {
      // Turn on All relevant Subsystem logging levels
      Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(FAULTMGRLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(DATAMGRLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(RSRCMONLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(MSGMGRLOG, DEVELOPERLOG);
   }//end if

   // Initialize the OPM
   OPM::initialize();

   // Initialize the Fault Manager client interface
   Faults::getInstance()->initialize();

   // Create the ResourceMonitor instance
   ResourceMonitor* resourceMonitor = new ResourceMonitor();
   if (!resourceMonitor)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Could not create ResourceMonitor instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Initialize the ResourceMonitor instance
   if (resourceMonitor->initialize(argc, argv) == ERROR)
   {
      TRACELOG(DEBUGLOG, RSRCMONLOG, "ResourceMonitor failed initialization",0,0,0,0,0,0);
      exit(-1);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, RSRCMONLOG, "ResourceMonitor successfully started with PID=%d", ACE_OS::getpid(),0,0,0,0,0);
   }//end else

   // Start ResourceMonitor's mailbox processing loop to handle notifications. Here we block
   // and run forever until we are shutdown by the SIGINT/SIGQUIT signal
   resourceMonitor->processMailbox();

   TRACELOG(ERRORLOG, RSRCMONLOG, "ResourceMonitor Main method exiting",0,0,0,0,0,0);
}//end main
