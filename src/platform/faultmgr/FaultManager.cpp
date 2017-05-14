/******************************************************************************
*
* File name:   FaultManager.cpp
* Subsystem:   Platform Services
* Description: Fault Manager is responsible for alarm and event report
*              collection on each node and forwarding to the EMS. Fault Manager
*              maintains a list of outstanding alarms in the local database.
*
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
#include <ace/Process_Semaphore.h>
#include <ace/Select_Reactor.h>
#include <ace/Sig_Adapter.h>
#include <ace/Time_Value.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "FaultManager.h"
#include "FaultMessage.h"

#include "platform/common/AlarmsEvents.h"

#include "platform/datamgr/DataManager.h"
#include "platform/datamgr/DbConnectionHandle.h"
#include "platform/common/ConnectionSetNames.h"

#include "platform/common/MailboxNames.h"
#include "platform/common/MessageIds.h"
#include "platform/msgmgr/DistributedMailbox.h"
#include "platform/msgmgr/MailboxAddress.h"
#include "platform/msgmgr/MailboxHandle.h"
#include "platform/msgmgr/MailboxLookupService.h"
#include "platform/msgmgr/MailboxOwnerHandle.h"
#include "platform/msgmgr/MailboxProcessor.h"
#include "platform/msgmgr/MessageBase.h"
#include "platform/msgmgr/MessageFactory.h"
#include "platform/msgmgr/MessageHandlerList.h"
#include "platform/msgmgr/TimerMessage.h"

#include "platform/messages/AlarmEventMessage.h"

#include "platform/logger/Logger.h"

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
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)FaultManager"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// Some seconds to delay when shutting down
#define SHUTDOWN_DELAY 2

// Default Fault reporting period for outstanding alarms (to tracelogs) in seconds
//#define DEFAULT_FAULT_REPORT_PERIOD 300 
#define DEFAULT_FAULT_REPORT_PERIOD 5 

// Time the Fault Message should stay in the Clear Alarm Cache before being sent
// to the EMS
#define CLEAR_ALARM_CACHE_TIME 5

// Static singleton instance
FaultManager::FaultManager* FaultManager::faultManager_ = NULL;

// Static Select Reactor instance
ACE_Reactor* FaultManager::selectReactor_ = NULL;

// Shutdown flag for notifying looping threads
bool FaultManager::shuttingDown_ = false;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
FaultManager::FaultManager()
               :faultManagerMailbox_(NULL),
                messageHandlerList_(NULL),
                emsMailboxHandle_(NULL),
                faultSMQueue_(FAULTSM_QUEUENAME, FAULTSM_QUEUEMUTEXNAME),
                processSemaphore_ (NULL)
{
   // Populate the static singleton instance
   faultManager_ = this;

   // Create/map the process semaphore. Initialize it to be blocked
   processSemaphore_ = new ACE_Process_Semaphore(0, FAULTQUEUE_SEMAPHORE_NAME);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
FaultManager::~FaultManager()
{
   // Deallocate/remove the process semaphore
   processSemaphore_->remove();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Catch the shutdown signal and prepare to graceful stop
// Design:
//-----------------------------------------------------------------------------
void FaultManager::catchShutdownSignal()
{
   TRACELOG(DEBUGLOG, FAULTMGRLOG, "Requested FaultManager shutdown",0,0,0,0,0,0);
   FaultManager::getInstance()->shutdown();
}//end catchShutdownSignal


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Retrieve the static singleton instance
// Design:
//-----------------------------------------------------------------------------
FaultManager* FaultManager::getInstance()
{
   if (faultManager_ == NULL)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Singleton instance is NULL",0,0,0,0,0,0);
   }//end if
   return faultManager_;
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Gracefully shutdown the Application and all of its child threads
// Design:
//-----------------------------------------------------------------------------
void FaultManager::shutdown()
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
void FaultManager::startReactor()
{
   // Set Reactor thread ownership
   selectReactor_->owner (ACE_Thread::self ());
   // Start the reactor processing loop
   while ( (selectReactor_->reactor_event_loop_done () == 0) && (shuttingDown_ == false) )
   {
      int result = selectReactor_->run_reactor_event_loop ();

      char errorBuff[200];
      char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
      if (resultStr == NULL)
      {
         TRACELOG(ERRORLOG, FAULTMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, FAULTMGRLOG, ostr.str().c_str());
   }//end while
}//end startReactor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialization method
// Design:
//-----------------------------------------------------------------------------
int FaultManager::initialize(int faultReportPeriod)
{
   // Initialize the DataManager and activate database connections first!
   if (DataManager::initialize("./DataManager.conf") == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error initializing Data manager",0,0,0,0,0,0);
      // To allow for development, do not return
      // return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully initialized Data Manager",0,0,0,0,0,0);
   }//end else

   // Activate the Fault Connection Set
   if (DataManager::activateConnectionSet(FAULT_MANAGER_CONNECTION) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error activating Fault ConnectionSet",0,0,0,0,0,0);
      // To allow for development, do not return
      // return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully activated Fault ConnectionSet",0,0,0,0,0,0);
   }//end else

   // Create the Distributed Mailbox Address
   MailboxAddress distributedAddress;
   distributedAddress.locationType = DISTRIBUTED_MAILBOX;
   distributedAddress.mailboxName = FAULT_MANAGER_MAILBOX_NAME;
   distributedAddress.inetAddress.set(FAULT_MANAGER_MAILBOX_PORT, LOCAL_IP_ADDRESS);
   distributedAddress.neid = SystemInfo::getLocalNEID();

   // Setup the distributed Mailbox
   if (!setupMailbox(distributedAddress))
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Unable to setup distributed mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "FaultManager distributed mailbox setup successful",0,0,0,0,0,0);
   }//end else

   // Setup the EMS Mailbox Address
   // NOTE: currently, finding a mailbox reference in the lookup service depends on
   // matching the locationType, mailbox name, and IP Address and port
   emsMailboxAddress_.locationType = DISTRIBUTED_MAILBOX;
   emsMailboxAddress_.mailboxName = CLIENT_AGENT_MAILBOX_NAME;
   emsMailboxAddress_.inetAddress.set(CLIENT_AGENT_MAILBOX_PORT, LOCAL_IP_ADDRESS);

   // Get a handle to the EMS Mailbox
   emsMailboxHandle_ = MailboxLookupService::find(emsMailboxAddress_);
   if (!emsMailboxHandle_)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Lookup Service find on EMS Proxy Mailbox returned NULL",0,0,0,0,0,0);
   }//end if

   // Create a new ACE_Select_Reactor for the signal handling, etc to use
   selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create an adapter to shutdown this service and all children
   signalAdapter_ = new ACE_Sig_Adapter((ACE_Sig_Handler_Ex) FaultManager::catchShutdownSignal);

   // Specify which types of signals we want to trap
   signalSet_.sig_add (SIGINT);
   signalSet_.sig_add (SIGQUIT);
   signalSet_.sig_add (SIGTERM);

   // Register ourselves to receive signals so we can shut down gracefully.
   if (selectReactor_->register_handler (signalSet_, signalAdapter_) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error registering for OS signals",0,0,0,0,0,0);
   }//end if

   // Startup the Reactor processing loop in a dedicated thread. Here, returns the OS assigned
   // unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)FaultManager::startReactor, (void*) 0, "FaultManagerReactor", true);

   // Setup shared memory
   if (faultSMQueue_.setupQueue() == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error setting up shared memory queue",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Startup to the Alarm processing loop in another dedicated thread. Here, returns
   // OS assigned unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)FaultManager::processAlarms, (void*) 0, "FaultManagerProcessAlarms", true);

   // Startup the Clear Alarm Cache processing loop in a dedicated thread. Here, we check
   // to ensure that a toggling (on,off,on,off) Alarm condition does not exist before sending
   // the Clear Alarm to the EMS.
   ThreadManager::createThread((ACE_THR_FUNC)FaultManager::startClearAlarmCache, (void*) 0, "FaultManagerClearAlarmCache", true);

   // Start the timer to periodically generate the fault reports (use the passed in interval
   // parameter; otherwise default the value). Setting to '0' disables this functionality
   if (faultReportPeriod != 0)
   {
      // Create a Timer Message and schedule it. Here we are specifying a restartInterval,
      ACE_Time_Value timerDelay(faultReportPeriod);
      ACE_Time_Value restartInterval(faultReportPeriod);
      TimerMessage* timerMessage = new TimerMessage(distributedAddress, 1 /*version*/, timerDelay, restartInterval);

      // Schedule the Timer Message with the Fault Manager Local mailbox. Will we ever need to 
      // cancel this timer?? For now, assume no.
      long timerId = faultManagerMailbox_->scheduleTimer(timerMessage);
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "Scheduled a timer with Id %ld",timerId,0,0,0,0,0);
   }//end if

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void FaultManager::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *faultManagerMailbox_);

   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (faultManagerMailbox_->activate() == ERROR)
   {
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the remote side. Applications need to retry.
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Failed to activate distributed mailbox",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Timer Messages. Here we generate the fault report
// Design:
//-----------------------------------------------------------------------------
int FaultManager::processAlarmReportTimerMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   //TimerMessage* timerMessage = (TimerMessage*) message;
   TRACELOG(DEBUGLOG, FAULTMGRLOG, "Preparing fault report:",0,0,0,0,0,0);

   // Reserve the database connection
   DbConnectionHandle* connectionHandle = DataManager::reserveConnection(FAULT_MANAGER_CONNECTION);
   if (connectionHandle == NULL)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error reserving connection",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully reserved Fault ConnectionSet Connection",0,0,0,0,0,0);
   }//end else

   // Run a query against the local AlarmsOutstanding table
   string outstandingAlarmsQuery = "Select NEID,AlarmCode,ManagedObject,ManagedObjectInstance,Pid,TimeStamp from platform.localAlarmsOutstanding";

   // Execute the query against the database
   if (connectionHandle->executeCommand(outstandingAlarmsQuery) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Local AlarmsOutstanding query execution failed",0,0,0,0,0,0);
      DataManager::releaseConnection(connectionHandle);
      return ERROR;
   }//end if

   // Build the outstanding alarms report and display in the tracelogs
   TRACELOG(DEBUGLOG, FAULTMGRLOG, "--------- Outstanding Alarms Report ---------",0,0,0,0,0,0);

   int columnCount = connectionHandle->getColumnCount();
   int rowCount = connectionHandle->getRowCount();
   // Do some checks to see if the number of columns returned is zero! This indicates a schema error!
   if (columnCount == 0)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "No columns returned in result set, possible schema error",0,0,0,0,0,0);
      connectionHandle->closeCommandResult();
      DataManager::releaseConnection(connectionHandle);
      return ERROR;
   }//end if
   else if (rowCount == 0)
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "No alarms outstanding",0,0,0,0,0,0);
   }//end if
   else
   {
      // Get the column Indexes
      int neidColumnIndex = connectionHandle->getColumnIndex("NEID");
      int alarmCodeColumnIndex = connectionHandle->getColumnIndex("AlarmCode");
      int managedObjectColumnIndex = connectionHandle->getColumnIndex("ManagedObject");
      int managedObjectInstanceColumnIndex = connectionHandle->getColumnIndex("ManagedObjectInstance");
      int pidColumnIndex = connectionHandle->getColumnIndex("Pid");
      int timeStampColumnIndex = connectionHandle->getColumnIndex("TimeStamp");

      // Display the results in a Table-style format. FIRST, display all of the column headers in a log
      string rowBuffer("      ");
      for (int column = 0; column < columnCount; column++)
      {
         rowBuffer = rowBuffer + " " + connectionHandle->getColumnName(column);
      }//end for
      STRACELOG(DEBUGLOG, FAULTMGRLOG, rowBuffer.c_str());
 
      for (int row = 0; row < rowCount; row++)
      {
         ostringstream ostr;
         ostr << "Row" << row << "  ";
         for (int column = 0; column < columnCount; column++)
         {
            // We have to use the type-specific retrieval methods, so need to identify each column
            if (column == neidColumnIndex)
            {
               ostr << "(" << connectionHandle->getStringValueAt(row, column) << ")  ";
               continue;
            }//end if
            else if ( (column == alarmCodeColumnIndex) || (column == managedObjectColumnIndex) )
            {
               ostr << "(" << connectionHandle->getShortValueAt(row, column) << ")  ";
               continue;
            }//end else if
            else if ( (column == managedObjectInstanceColumnIndex) || (column == pidColumnIndex) )
            {
               ostr << "(" << connectionHandle->getIntValueAt(row, column) << ")  ";
               continue;
            }//end else if
            else if (column == timeStampColumnIndex)
            {
               ostr << "(" << connectionHandle->getTimestampValueAt(row, column) << ")  ";
               continue;
            }//end else if 
         }//end for
         ostr << ends;
         STRACELOG(DEBUGLOG, FAULTMGRLOG, ostr.str().c_str());
      }//end for
   }//end else

   // Display a tag to show that the list is complete
   TRACELOG(DEBUGLOG, FAULTMGRLOG, "---------     Alarms Report End     ---------",0,0,0,0,0,0);

   // Close the results, release the connection
   connectionHandle->closeCommandResult();
   DataManager::releaseConnection(connectionHandle);

   return OK;
}//end processAlarmReportTimerMessage


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Start the alarm processing loop
// Design:
//-----------------------------------------------------------------------------
void FaultManager::processAlarms()
{
   FaultMessage faultMessage;

   while ( (shuttingDown_ == false) || (FaultManager::getInstance()->faultSMQueue_.isEmpty() == false) )
   {
      // Retrieve the next alarm. This is a blocking call while the shared memory queue is empty
      if (FaultManager::getInstance()->getNextAlarm(faultMessage) == ERROR)
      {
         TRACELOG(ERRORLOG, FAULTMGRLOG, "Error retrieving fault message from shared memory",0,0,0,0,0,0);
      }//end if

      // Process the contents of the alarm
      FaultManager::getInstance()->processAlarmContents(faultMessage);
   }//end while
}//end processAlarms


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Start the Clear Alarm Cache processing thread
// Design:
//-----------------------------------------------------------------------------
void FaultManager::startClearAlarmCache()
{
   FaultManager::getInstance()->processClearAlarmCache();
}//end startClearAlarmCache


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Get the next alarm from the queue. This method will block while
//              the queue is empty.
// Design:
//-----------------------------------------------------------------------------
int FaultManager::getNextAlarm(FaultMessage& faultMessage)
{
   // Block here while the queue is empty
   while (faultSMQueue_.isEmpty() == true)
   {
      //cout << "Sleeping for 1 second while fault queue is empty" << endl;
      //ACE_OS::sleep(1);
   
      // Instead of doing a busy wait, let's do a real wait until we are signaled
      // by an alarm/event that the queue is no longer empty. This will block as long
      // as the semaphore counter is 0; otherwise it will decrement the counter.
      // The post event will increment the counter (and thus unblock the wait)
      processSemaphore_->acquire();
   }//end while

   return faultSMQueue_.dequeueAlarm(faultMessage);
}//end getNextAlarm


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Periodically loop through the Clear Alarm Cache. If a Clear Alarm has
//    survived without being canceled for the specified time, then send it to
//    the EMS.
// Design:
//-----------------------------------------------------------------------------
void FaultManager::processClearAlarmCache()
{
   while (shuttingDown_ == false)
   {
      clearAlarmCacheMutex_.acquire();

      ClearAlarmCacheVector::iterator cacheIterator = clearAlarmCache_.begin();
      ClearAlarmCacheVector::iterator endIterator = clearAlarmCache_.end();

      // Loop through the vector from beginning to end (since we always add the
      // newest to the end) looking for a Clear Alarm that has met the time
      // requirement to be sent to EMS
      while (cacheIterator != endIterator) 
      {
         FaultMessage* faultMessage = &(*cacheIterator);

         // Difference between time now and time FaultMessage was put in the cache / Originated
         // Note that we are assuming that the time for the FaultMessage to get originated and move
         // through shared memory and get inserted into the cache is negligable.
         if (time(0) - faultMessage->timeStamp >= CLEAR_ALARM_CACHE_TIME) 
         {
            TRACELOG(DEVELOPERLOG, FAULTMGRLOG, "Clear Alarm has reached cache expiration, preparing to send",0,0,0,0,0,0);

            // Remove the Alarm that the Clear Alarm applies to from the localAlarmsOutstanding list

            // Since this fault messages does clear an existing, outstanding alarm, then build a delete
            // statement to remove that alarm record from the local outstanding alarms table, and send the
            // clear alarm to the EMS. NOTE: The EMS SHOULD NOT completely delete the Alarm as it needs
            // to maintain a history of the acknowledgements, etc.
            ostringstream outstandingAlarmsDelete;
            outstandingAlarmsDelete << "Delete from platform.localAlarmsOutstanding where "
               << "NEID=" << faultMessage->neidPI << " and AlarmCode=" << faultMessage->alarmCode
               << " and ManagedObject=" << faultMessage->managedObject
               << " and ManagedObjectInstance=" << faultMessage->managedObjectInstance
               << " and Pid=" << faultMessage->pid << ";" << ends;

            // Reserve the database connection
            DbConnectionHandle* connectionHandle = DataManager::reserveConnection(FAULT_MANAGER_CONNECTION);
            if (connectionHandle == NULL)
            {
               TRACELOG(ERRORLOG, FAULTMGRLOG, "Error reserving connection",0,0,0,0,0,0);
               continue;
            }//end if
            else
            {
               TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully reserved Fault ConnectionSet Connection",0,0,0,0,0,0);
            }//end else

            // Execute the delete against the database
            if (connectionHandle->executeCommand(outstandingAlarmsDelete.str().c_str()) == ERROR)
            {
               TRACELOG(ERRORLOG, FAULTMGRLOG, "Local AlarmsOutstanding delete execution failed",0,0,0,0,0,0);
               DataManager::releaseConnection(connectionHandle);
               continue;
            }//end if
            else
            {
               TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully cleared alarm in local outstanding alarms list",0,0,0,0,0,0);
               DataManager::releaseConnection(connectionHandle);
            }//end else

            // Send the Clear Alarm to the EMS
            // Create an AlarmEventMessage to encapsulate alarms and events to the EMS
            char* neid = faultMessage->neidPI;
            AlarmEventMessage* alarmEventMessage = new AlarmEventMessage(faultManagerMailbox_->getMailboxAddress(),
               CLEAR_ALARM_MESSAGE, neid, faultMessage->managedObject, faultMessage->managedObjectInstance,
               faultMessage->alarmCode, faultMessage->eventCode, faultMessage->pid, faultMessage->timeStamp);

            // Post the AlarmEvent Message to the EMS' mailbox, first check to see if we have a valid handle
            if (!emsMailboxHandle_)
            {
               emsMailboxHandle_ = MailboxLookupService::find(emsMailboxAddress_);
            }//end if
            
            // If we now have a valid handle, then post the message
            if (emsMailboxHandle_)
            {
               // Remember that if post is successful, then the MsgMgr framework will delete it; otherwise,
               // it is the application's responsibility to delete it or retry
               if (emsMailboxHandle_->post(alarmEventMessage) != ERROR)
               {
                  TRACELOG(DEBUGLOG, FAULTMGRLOG, "Posted Clear Alarm message to the EMS",0,0,0,0,0,0);
               }//end if
               else
               {
                  delete emsMailboxHandle_;
                                                                                                                                 
                  if (!(emsMailboxHandle_ = MailboxLookupService::find(emsMailboxAddress_)))
                  {
                     TRACELOG(ERRORLOG, FAULTMGRLOG, "Failed to perform MLS re-find after message post failed",0,0,0,0,0,0);
                     delete alarmEventMessage;
                  }//end else
                  else if (emsMailboxHandle_->post(alarmEventMessage) == ERROR)
                  {
                     TRACELOG(ERRORLOG, FAULTMGRLOG, "Message post failed even after MLS re-find performed",0,0,0,0,0,0);
                     delete alarmEventMessage;
                  }//end else if
                  else
                  {
                     TRACELOG(WARNINGLOG, FAULTMGRLOG, "Initial Clear Alarm post to EMS failed, but worked after MLS re-find",0,0,0,0,0,0);
                  }//end else
               }//end else
            }//end if
            else
            {
               TRACELOG(ERRORLOG, FAULTMGRLOG, "Failed to send Clear Alarm; could not find EMS Mailbox",0,0,0,0,0,0);
            }//end else

            // Remove the Clear Alarm from the Cache (Vector will delete the FaultMessage)
            clearAlarmCache_.erase(cacheIterator);
         }//end if
         cacheIterator++;
      }//end while
      clearAlarmCacheMutex_.release();

      // Sleep for 2 seconds and check again
      sleep(2);
   }//end while
}//end processClearAlarmCache


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve and process the contents of the next fault message
// Design: This method gets called when an alarm/clear/event is detected and
//   retrieved from the shared memory queue.
//-----------------------------------------------------------------------------
void FaultManager::processAlarmContents(FaultMessage& faultMessage)
{
   switch (faultMessage.alarmSeverity)
   {
      case (ALARM_INDETERMINATE):
      {
         TRACELOG(DEVELOPERLOG, FAULTMGRLOG, "Received Alarm with id (%d)", faultMessage.alarmCode,0,0,0,0,0);

         // Massage the TimeStamp value to work in the database schema
         char timeBuffer[30];
         time_t timeStamp = faultMessage.timeStamp;
         strftime(timeBuffer, sizeof(timeBuffer), "%F %H:%M:%S.00%z", localtime(&timeStamp));
         // Truncate the time zone to 2 characters
         timeBuffer[strlen(timeBuffer) - 2] = '\0';

         // Build a query to see if this alarm has already been raised (and is in the local outstanding
         // alarms list in the database)
         ostringstream outstandingAlarmsQuery;
         outstandingAlarmsQuery << "Select AlarmCode from platform.localAlarmsOutstanding where"
           " NEID=" << faultMessage.neidPI << 
           " and AlarmCode=" << faultMessage.alarmCode << 
           " and ManagedObject=" << faultMessage.managedObject <<
           " and ManagedObjectInstance=" << faultMessage.managedObjectInstance <<
           " and Pid=" << faultMessage.pid << ";" << ends;

         // Reserve the database connection
         DbConnectionHandle* connectionHandle = DataManager::reserveConnection(FAULT_MANAGER_CONNECTION);
         if (connectionHandle == NULL)
         {
            TRACELOG(ERRORLOG, FAULTMGRLOG, "Error reserving connection",0,0,0,0,0,0);
            return;
         }//end if
         else
         {
            TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully reserved Fault ConnectionSet Connection",0,0,0,0,0,0);
         }//end else

         // Execute the query against the database
         if (connectionHandle->executeCommand(outstandingAlarmsQuery.str().c_str()) == ERROR)
         {
            TRACELOG(ERRORLOG, FAULTMGRLOG, "Local AlarmsOutstanding query execution failed",0,0,0,0,0,0);
            DataManager::releaseConnection(connectionHandle);
            return;
         }//end if

         // If this is a new alarm, then build a query to insert it in the outstanding alarms list 
         // and send the alarm to the EMS as well
         if (connectionHandle->getRowCount() != 0)
         {
            TRACELOG(DEBUGLOG, FAULTMGRLOG, "Throttling duplicate Alarm raised for AlarmId (%d)", faultMessage.alarmCode,0,0,0,0,0);
            // Release the connection/result set
            connectionHandle->closeCommandResult();
            DataManager::releaseConnection(connectionHandle);

            // Check to see if there was a Clear Alarm pending in the Clear Alarm Cache. If so,
            // remove the Clear Alarm from the cache and discard it
            clearAlarmCacheMutex_.acquire();
            ClearAlarmCacheVector::iterator cacheIterator = clearAlarmCache_.begin();
            ClearAlarmCacheVector::iterator endIterator = clearAlarmCache_.end();

            // Loop through the vector from beginning to end (since we always add the
            // newest to the end) looking for a Clear Alarm that matches this new Raised Alarm
            while (cacheIterator != endIterator)
            {
               FaultMessage* cachedMessage = &(*cacheIterator);
               if (ACE_OS::strcmp(faultMessage.neidPI, cachedMessage->neidPI) == 0 &&
                   faultMessage.managedObject == cachedMessage->managedObject &&
                   faultMessage.managedObjectInstance == cachedMessage->managedObjectInstance &&
                   faultMessage.alarmCode == cachedMessage->alarmCode &&
                   faultMessage.pid == cachedMessage->pid)
               {
                  TRACELOG(DEBUGLOG, FAULTMGRLOG, "Canceling and discarding Clear Alarm in Cache",0,0,0,0,0,0);
                  // Remove the Clear Alarm from the Cache (Here, vector will delete the stored FaultMessage)
                  clearAlarmCache_.erase(cacheIterator);
                  // Break out of the loop
                  break;
               }//end if
               cacheIterator++;
            }//end while
            clearAlarmCacheMutex_.release();

            // Return so that this 'Raise Alarm' is discarded without taking action
            return;
         }//end if
         else
         {
            ostringstream outstandingAlarmsInsert;
            // NOTE: Acknowledgement field is populated by the EMS operator, so not here
            outstandingAlarmsInsert << "Insert into platform.localAlarmsOutstanding " 
               << "(NEID, AlarmCode, ManagedObject, ManagedObjectInstance, Pid, TimeStamp) "
               << "values (" << faultMessage.neidPI << "," << faultMessage.alarmCode 
               << "," << faultMessage.managedObject << "," << faultMessage.managedObjectInstance 
               << "," << faultMessage.pid << ",'" << timeBuffer << "');" << ends;

            // Execute the insert against the database
            if (connectionHandle->executeCommand(outstandingAlarmsInsert.str().c_str()) == ERROR)
            {
               TRACELOG(ERRORLOG, FAULTMGRLOG, "Local AlarmsOutstanding insert execution failed",0,0,0,0,0,0);
               DataManager::releaseConnection(connectionHandle);
               return;
            }//end if
            else
            {
               TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully stored alarm in local outstanding alarms list",0,0,0,0,0,0);
               DataManager::releaseConnection(connectionHandle);
            }//end else

            // Send the Alarm to the EMS
            // Create an AlarmEventMessage to encapsulate alarms and events to the EMS
            char* neid = faultMessage.neidPI; // convert from template type
            AlarmEventMessage* alarmEventMessage = new AlarmEventMessage(faultManagerMailbox_->getMailboxAddress(),
               ALARM_MESSAGE, neid, faultMessage.managedObject, faultMessage.managedObjectInstance,
               faultMessage.alarmCode, faultMessage.eventCode, faultMessage.pid, faultMessage.timeStamp);

            // Post the AlarmEvent Message to the EMS' mailbox, first check to see if we have a valid handle
            if (!emsMailboxHandle_)
            {
               emsMailboxHandle_ = MailboxLookupService::find(emsMailboxAddress_);
            }//end if

            // If we now have a valid handle, then post the message
            if (emsMailboxHandle_)
            {
//TAKE OUT:
MailboxLookupService::setDebugForAllMailboxAddresses(10);
               // Remember that if post is successful, then the MsgMgr framework will delete it; otherwise,
               // it is the application's responsibility to delete it or retry
               if (emsMailboxHandle_->post(alarmEventMessage) != ERROR)
               {
                  TRACELOG(DEBUGLOG, FAULTMGRLOG, "Posted Alarm message to the EMS",0,0,0,0,0,0);
               }//end if
               else
               {
                  delete emsMailboxHandle_;

                  if (!(emsMailboxHandle_ = MailboxLookupService::find(emsMailboxAddress_)))
                  {
                     // Here, find failed so, the connection is bad and the handle ultimately needs to be deleted
                     // and retried again.
                     TRACELOG(ERRORLOG, FAULTMGRLOG, "Failed to perform MLS re-find after message post failed",0,0,0,0,0,0);
                     delete alarmEventMessage;
                  }//end else
                  else if (emsMailboxHandle_->post(alarmEventMessage) == ERROR)
                  {
                     TRACELOG(ERRORLOG, FAULTMGRLOG, "Message post failed even after MLS re-find performed",0,0,0,0,0,0);
                     delete alarmEventMessage;
                  }//end else if
                  else
                  {
                     TRACELOG(WARNINGLOG, FAULTMGRLOG, "Initial Alarm post to EMS failed, but worked after MLS re-find",0,0,0,0,0,0);
                  }//end else
               }//end else
            }//end if
            else
            {
               TRACELOG(ERRORLOG, FAULTMGRLOG, "Failed to raise Alarm; could not find EMS Mailbox",0,0,0,0,0,0);
            }//end else
         }//end else

         break;
      }//end raise alarm
      case (ALARM_CLEAR):
      {
         TRACELOG(DEVELOPERLOG, FAULTMGRLOG, "Received Clear Alarm with id (%d)",faultMessage.alarmCode,0,0,0,0,0);
   
         // Build a query to see if this fault message clears an alarm that has already been raised 
         // (and is in the outstanding alarms list in the database)
         ostringstream outstandingAlarmsQuery;
         outstandingAlarmsQuery << "Select AlarmCode from platform.localAlarmsOutstanding where"
           " NEID=" << faultMessage.neidPI <<
           " and AlarmCode=" << faultMessage.alarmCode <<
           " and ManagedObject=" << faultMessage.managedObject <<
           " and ManagedObjectInstance=" << faultMessage.managedObjectInstance <<
           " and Pid=" << faultMessage.pid << ";" << ends;

         // Reserve the database connection
         DbConnectionHandle* connectionHandle = DataManager::reserveConnection(FAULT_MANAGER_CONNECTION);
         if (connectionHandle == NULL)
         {
            TRACELOG(ERRORLOG, FAULTMGRLOG, "Error reserving connection",0,0,0,0,0,0);
            return;
         }//end if
         else
         {
            TRACELOG(DEBUGLOG, FAULTMGRLOG, "Successfully reserved Fault ConnectionSet Connection",0,0,0,0,0,0);
         }//end else

         // Execute the query against the database
         if (connectionHandle->executeCommand(outstandingAlarmsQuery.str().c_str()) == ERROR)
         {
            TRACELOG(ERRORLOG, FAULTMGRLOG, "Local AlarmsOutstanding query execution failed",0,0,0,0,0,0);
            DataManager::releaseConnection(connectionHandle);
            return;
         }//end if

         // If this is a new alarm, then build a query to insert it in the outstanding alarms list
         // and send the alarm to the EMS as well
         if (connectionHandle->getRowCount() == 0)
         {
            TRACELOG(DEBUGLOG, FAULTMGRLOG, "Throttling duplicate Clear Alarm for AlarmId (%d)", 
               faultMessage.alarmCode,0,0,0,0,0);
            connectionHandle->closeCommandResult();
            DataManager::releaseConnection(connectionHandle);
         }//end if
         else
         {
            // Release the connection
            DataManager::releaseConnection(connectionHandle);
            // Store the Clear Alarm in the Cache with the current timeStamp.
            clearAlarmCacheMutex_.acquire();
            // Insert into the end of the cache vector (Vector will create a copy of the FaultMessage)
            clearAlarmCache_.push_back(faultMessage);
            clearAlarmCacheMutex_.release();
         }//end else
         break;
      }//end clear alarm
      case (INFORMATIONAL_EVENT):
      {
         TRACELOG(DEVELOPERLOG, FAULTMGRLOG, "Received event report with id (%d)",faultMessage.eventCode,0,0,0,0,0);

         // We have 3 options for developing event reports:
         // a.) We can write event reports to the syslog facility which can be redirected to the EMS.
         //     If we do this, EMS clients will have to remotely view syslog files
         // b.) Write event reports to the database, (remote or local), the EMS can extract and remove them
         // c.) Send event reports to the EMS via some IPC.

         // For now, we send the event reports to the EMS' msgmgr mailbox
         // Create an AlarmEventMessage to encapsulate alarms and events to the EMS
         char* neid = faultMessage.neidPI; // convert from template type
         AlarmEventMessage* alarmEventMessage = new AlarmEventMessage(faultManagerMailbox_->getMailboxAddress(),
            EVENT_REPORT_MESSAGE, neid, faultMessage.managedObject, faultMessage.managedObjectInstance,
            faultMessage.alarmCode, faultMessage.eventCode, faultMessage.pid, faultMessage.timeStamp);

         // Post the AlarmEvent Message to the EMS' mailbox, first check to see if we have a valid handle
         if (!emsMailboxHandle_)
         {
            emsMailboxHandle_ = MailboxLookupService::find(emsMailboxAddress_);
         }//end if

         // If we now have a valid handle, then post the message
         if (emsMailboxHandle_)
         {
            // Remember that if post is successful, then the MsgMgr framework will delete it; otherwise,
            // it is the application's responsibility to delete it or retry
            if (emsMailboxHandle_->post(alarmEventMessage) != ERROR)
            {
               TRACELOG(DEBUGLOG, FAULTMGRLOG, "Posted Event Report message to the EMS",0,0,0,0,0,0);
            }//end if
            else
            {
               delete emsMailboxHandle_;

               if (!(emsMailboxHandle_ = MailboxLookupService::find(emsMailboxAddress_)))
               {
                  // Here, find failed so, the connection is bad and the handle ultimately needs to be deleted
                  // and retried again.
                  TRACELOG(ERRORLOG, FAULTMGRLOG, "Failed to perform MLS re-find after message post failed",0,0,0,0,0,0);
                  delete alarmEventMessage;
               }//end else
               else if (emsMailboxHandle_->post(alarmEventMessage) == ERROR)
               {
                  TRACELOG(ERRORLOG, FAULTMGRLOG, "Message post failed even after MLS re-find performed",0,0,0,0,0,0);
                  delete alarmEventMessage;
               }//end else if
               else
               {
                  TRACELOG(WARNINGLOG, FAULTMGRLOG, "Initial Event Report post to EMS failed, but worked after MLS re-find",0,0,0,0,0,0);
               }//end else
            }//end else
         }//end if
         else
         {
            TRACELOG(ERRORLOG, FAULTMGRLOG, "Failed to send Event Report; could not find EMS Mailbox",0,0,0,0,0,0);
         }//end else
         break;
      }//end event report
      default:
      {
         TRACELOG(ERRORLOG, FAULTMGRLOG, "Unknown fault message type dequeued, severity: %d",
            faultMessage.alarmSeverity,0,0,0,0,0);
         break;
      }//end default
   }//end switch
}//end processAlarmContents


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup the Mailbox framework
// Design:
//-----------------------------------------------------------------------------
bool FaultManager::setupMailbox(const MailboxAddress& distributedAddress)
{
   // Create the Local Mailbox
   faultManagerMailbox_ = DistributedMailbox::createMailbox(distributedAddress);
   if (!faultManagerMailbox_)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Unable to create FaultManager mailbox",0,0,0,0,0,0);
      return false;
   }//end if

//TAKE OUT:
faultManagerMailbox_->setDebugValue(10);
MailboxLookupService::setDebugForAllMailboxAddresses(10);

   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Unable to create message handler list",0,0,0,0,0,0);
      return false;
   }//end if

   // Create handlers for each message expected
   MessageHandler timerMessageHandler = makeFunctor((MessageHandler*)0,
                                        *this, &FaultManager::processAlarmReportTimerMessage);

   // Add the message handlers to the message handler list to be used by the MsgMgr
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor

   // For now, use the MsgMgr Base Timer -- no need to derive our own since we have no parameters
   messageHandlerList_->add(MSGMGR_BASE_TIMER_ID, timerMessageHandler);

   // Register support for distributed messages so that the DistributedMessageFactory
   // knows how to recreate them in 'MessageBase' form when they are received.

   return true;
}//end setupMailbox


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: main function for FaultManager binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
   int faultReportPeriod = DEFAULT_FAULT_REPORT_PERIOD;
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
   usageString << "Usage:\n  FaultManager\n" << str4spaces 
               /* command line options */
               << "-r <seconds> Interval period for which Fault Manager will publish a "
               <<      "fault report of outstanding alarms in the trace logs\n" << str4spaces
               << "-l Local Logger Mode only (do not enqueue logs to shared memory)\n" << str4spaces;
   usageString << ends;

   // Perform our own arguments parsing -before- we pass args to the class
   ACE_Get_Opt get_opt (argc, argv, "lr:");
   int c;
   while ((c = get_opt ()) != ERROR)
   {
      switch (c)
      {
         case 'r':
         {
            // get the time between publishing fault reports to trace logs
            faultReportPeriod = ACE_OS::atoi(get_opt.optarg);
            break; 
         }//end case
         case 'l':
         {
            // set local logger mode (don't enqueue to shared memory log processor)
            localLoggerMode = true;
            break;
         }//end case
         default:
            cout << "Misunderstood option: " << c << endl;
            cout << usageString.str() << endl;
            exit(ERROR);
      }//end switch
   }//end while

   // Initialize the Logger such that logs are enqueued to shared memory and can
   // be extracted by the LogProcessor (unless local logger mode command line option
   // was specified)
   Logger::getInstance()->initialize(localLoggerMode);
   if (localLoggerMode)
   {
      // Turn on/off All relevant Subsystem logging levels
      Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(FAULTMGRLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(DATAMGRLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(MSGMGRLOG, DEVELOPERLOG);
   }//end if

   // Initialize the OPM
   OPM::initialize();

   // Create the FaultManager instance
   FaultManager* faultManager = new FaultManager();
   if (!faultManager)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Could not create FaultManager instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Initialize the FaultManager instance
   if (faultManager->initialize(faultReportPeriod) == ERROR)
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "FaultManager failed initialization",0,0,0,0,0,0);
      exit(ERROR);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "FaultManager successfully started with PID=%d", ACE_OS::getpid(),0,0,0,0,0);
   }//end else

   // Start FaultManager's mailbox processing loop to handle notifications. Here we block
   // and run forever until we are shutdown by the SIGINT/SIGQUIT signal
   faultManager->processMailbox();

   TRACELOG(ERRORLOG, FAULTMGRLOG, "Main method exiting",0,0,0,0,0,0);
}//end main
