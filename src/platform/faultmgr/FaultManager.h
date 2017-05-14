/******************************************************************************
* 
* File name:   FaultManager.h 
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

#ifndef _PLAT_FAULT_MANAGER_H_
#define _PLAT_FAULT_MANAGER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>
#include <vector>

#include <ace/Reactor.h>
#include <ace/Signal.h>
#include <ace/Thread_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "FaultSMQueue.h"

#include "platform/msgmgr/MailboxAddress.h"

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

// For C++ class declarations, we have one (and only one) of these access 
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors/destructors first)
// 5) instance data
//

/**
 * The Fault Manager is responsible for alarm and event report
 * collection on each node and forwarding to the EMS. Fault Manager
 * maintains a list of outstanding alarms in the local database.
 * <p>
 * For Alarms, Fault Manager checks the outstanding alarm list in the
 * local database. If the alarm is not already raised, then it adds it to 
 * the database and sends it via the msgmgr framework to the EMS. Similarly for
 * clear alarms, if an outstanding alarm does not exist for the clear alarm, then
 * the clear is discarded and NOT sent to the EMS. This logic avoids repeated
 * alarms being sent to the EMS. 
 * <p>
 * For Automatically Clearing Alarms, the Clear Alarm is stored in cache for
 * 5 seconds. If a 'matching' Raised Alarm occurs that would negate the Clear
 * Alarm within the 5 second window, then the new Raised Alarm -and- the cached
 * Clear Alarm will be discarded (and nothing propagated up to the EMS). Only
 * after a 5 second time in which NO new matching Raised Alarms have occurred would
 * the Clear Alarm be allowd to propagate up to the EMS. This is to prevent
 * toggling (raised, cleared, raised, cleared) Alarm storms from being propagated
 * up to the EMS.
 * <p>
 * For Manually Clearing Alarms, the EMS is required to remove the Manual Alarm
 * from the node's localAlarmsOutstanding database table when they are manually
 * cleared via the UI. In this way, manually clearing Alarms can be subjected
 * to the same duplicate Raised Alarm type throttling as automatically clearing
 * alarms.
 * <p>
 * Fault Manager's interface supports being able to Raise/Clear alarms on behalf
 * of another entity/node. For example, if Cards X dies, then Card Y who is monitoring
 * it can Raise an Alarm on behalf of Card X (such that the alarm will apply to Card X
 * in the EMS). This is done by specifying the NEID in the Raise/Clear macro calls.
 * <p>
 * We have three options for how event reports are processed (not yet implemented):
 * a.) We can write event reports to the syslog facility which can be redirected to the EMS.
 *     If we do this, EMS clients will have to remotely view syslog files
 * b.) Write event reports to the database, (remote or local), the EMS can extract and remove them
 * c.) Send event reports to the EMS via some IPC.
 * <p>
 * FaultManager periodically generates a report/list of the local outstanding alarms 
 * in the trace logs. The period for this report is governed by a startup option (see below).
 * <p>
 * FaultManager accepts the following startup options:
 * -r <seconds> Interval period for which Fault Manager will publish a fault report
 *      of outstanding alarms in the trace logs (0 disables this functionality, if omitted,
 *      it will default to some value)
 * -l Local Logger Mode only (do not enqueue logs to shared memory)
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ACE_Process_Semaphore;
class FaultMessage;
class MailboxHandle;
class MailboxOwnerHandle;
class MessageBase;
class MessageHandlerList;

class FaultManager
{
   /**
    * Vector is used to store the Clear Alarms for de-bouncing
    * When storing object instances inside STL containers, the container will
    * handle making a copy of the object for storage (Copy Constructor required).
    * If we choose to store pointers to objects inside a container, then
    * we are responsible for deletion of the object OFF OF THE HEAP when we
    * remove it from the container
    */
   typedef vector<FaultMessage> ClearAlarmCacheVector;

   public:

      /** Constructor */
      FaultManager();

      /** Virtual Destructor */
      virtual ~FaultManager();

      /** Start the mailbox processing loop */
      void processMailbox();

      /** Catch the shutdown signal and begin the graceful shutdown */
      static void catchShutdownSignal();

      /** 
       * Perform FaultManager initialization
       * @param faultReportPeriod Interval period for which Fault Manager will publish a 
       *         fault report of outstanding alarms in the trace logs
       * @returns ERROR on failure; otherwise OK
       */
      int initialize(int faultReportPeriod);

   protected:

   private:

      /** Retrieve the static singleton instance of the FaultManager */
      static FaultManager* getInstance();

      /** Start the reactor processing thread for signal handling, etc. */
      static void startReactor();

      /** Start the Clear Alarm Cache processing thread */
      static void startClearAlarmCache();

      /**
       * Start the alarm processing loop. This method blocks forever.
       */
      static void processAlarms();

      /**
       * Periodically loop through the Clear Alarm Cache. If a Clear Alarm has
       * survived without being canceled for the specified time, then send it to
       * the EMS.
       */
      void processClearAlarmCache();

      /**
       * Process the contents of each alarm/clear/event report. This is used for normal runtime
       * processing of alarms as well as the flush procedure that is performed
       * at shutdown for clearing out the queue.
       */
      void processAlarmContents(FaultMessage& faultMessage);

      /**
       * Get the next alarm from the queue. This method will block while
       * the queue is empty.
       * @returns OK on success; otherwise ERROR
       */
      int getNextAlarm(FaultMessage& faultMessage);
 
      /**
       * Message Handler for Timer Messages. Generates fault report in trace logs.
       */
      int processAlarmReportTimerMessage(MessageBase* message);

      /** Gracefully shutdown the FaultManager and all of its threads */
      void shutdown();

      /**
       * Called in the constructor to setup the mailbox, activate and
       * to setup handlers.
       * @param remoteAddress Mailbox Address of the FaultManager DistributedMailbox
       * @returns true if successful
       */
      bool setupMailbox(const MailboxAddress& remoteAddress);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      FaultManager(const FaultManager& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      FaultManager& operator= (const FaultManager& rhs);

      /** Static singleton instance */
      static FaultManager* faultManager_;

      /** Flag for indicating when we are shutting down the process */
      static bool shuttingDown_;

      /** FaultManager Mailbox */
      MailboxOwnerHandle* faultManagerMailbox_;

      /** Message Handler List for storing Functors for Mailbox handlers */
      MessageHandlerList* messageHandlerList_;

      /** Mailbox Address for the EMS' Mailbox */
      MailboxAddress emsMailboxAddress_;

      /** Handle to the EMS' Mailbox */
      MailboxHandle* emsMailboxHandle_;

      /** Signal Adapter for registering signal handlers */
      ACE_Sig_Adapter* signalAdapter_;

      /** Signal Set for registering signal handlers */
      ACE_Sig_Set signalSet_;

      /** Shared Memory Queue for dequeuing fault messages / alarms from the applications */
      FaultSMQueue faultSMQueue_;

      /**
       * Vector for storing/caching Clear Alarms in while waiting to determine if there is
       * a toggling (on, off, on, off) alarm condition
       */
      ClearAlarmCacheVector clearAlarmCache_;

      /**
       * Non-recursive Thread mutex to protect the Clear Alarm Cache Vector
       */
      ACE_Thread_Mutex clearAlarmCacheMutex_;

      /** ACE Process Semaphore used to signal between enqueue/dequeue threads/processes
          when the queue is empty and non-empty */
      ACE_Process_Semaphore* processSemaphore_;

      /** ACE_Select_Reactor used for signal handling */
      static ACE_Reactor* selectReactor_;
};

#endif
