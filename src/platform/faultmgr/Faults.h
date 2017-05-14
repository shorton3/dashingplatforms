/******************************************************************************
*
* File name:   Faults.h
* Subsystem:   Platform Services
* Description: Implements the main API interface for raising and clearing
*              alarms and generating Information Event Reports. This interface
*              implements a high performance queuing mechanism for handling alarms.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/
                                                                                                                
#ifndef _PLAT_FAULTS_H_
#define _PLAT_FAULTS_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

/** Needed for Alarm severity and managed object Declarations */
#include "platform/common/AlarmsEvents.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Use C Linkage to prevent problems with the MACROS being name mangled
//#ifdef __cplusplus
//extern "C" {
//#endif

/** This is the programmatical interface used for raising alarms.
    Provides support for Raising Alarms on behalf of another NEID */
#define RAISEALARM(neid, alarmCode, managedObject, managedObjectInstance)                                         \
   TRACELOG(ERRORLOG, FAULTMGRLOG, "Raising Alarm %d instance %d", (alarmCode),(managedObjectInstance),0,0,0,0);  \
   Faults::raiseAlarm( (neid), (alarmCode), (managedObject), (managedObjectInstance), (unsigned int)(getpid()) ); 

/** Interface for clearing alarms. Provides support for Clearing Alarms on behalf of another NEID */
#define CLEARALARM(neid, alarmCode, managedObject, managedObjectInstance)                                         \
   TRACELOG(INFOLOG, FAULTMGRLOG, "Clearing Alarm %d instance %d", (alarmCode),(managedObjectInstance),0,0,0,0); \
   Faults::clearAlarm( (neid), (alarmCode), (managedObject), (managedObjectInstance), (unsigned int)(getpid()) );

/** Interface for reporting information events */
#define REPORTEVENT(eventCode, managedObject, managedObjectInstance) \
   TRACELOG(DEBUGLOG, FAULTMGRLOG, "Reporting Informational Event %d instance %d", (eventCode),(managedObjectInstance),0,0,0,0); \
   Faults::reportEvent( (eventCode), (managedObject), (managedObjectInstance), (unsigned int)(getpid()) );

//#ifdef __cplusplus
//}
//#endif

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

class ACE_Process_Semaphore;
class FaultMessage;
class FaultSMQueue;
                                                                                                                        
// For C++ class declarations, we have one (and only one) of these access
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors/destructors first)
// 5) instance data
                                                                                                                        
/**
 * Faults provides the main mechanism and API to the developers for raising
 * and clearing alarms and generating informational event reports.
 * <p>
 * Developers use the Fault MACROS to issue alarms. These macros hide the API
 * mechanism for enqueuing the alarms for output. In this way, the fault manager 
 * subsystem can be transparently modified in the future to include additional
 * data without impact to the application developers.
 * <p>
 * The key goal for implementing and maintaining this fault interface is to allow
 * applications to issue alarms with the MINIMAL amount of buffer copying
 * and alarm processing as possible. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class Faults
{
   public:
      
      /**
       * Destructor
       */
      ~Faults();

      /**
       * Method to handle enqueuing/raising of the alarms.
       * Supports being able to raise Alarms on behalf of another NEID/node.
       */
      static void raiseAlarm(const char* neid, AlarmCodeType alarmcode, ManagedObjectType managedObject, 
         unsigned int managedObjectInstance, unsigned int pid);

      /**
       * Method to handle enqueuing/clearing of the alarms.
       * Supports being able to clear Alarms on behalf of another NEID/node.
       */
      static void clearAlarm(const char* neid, AlarmCodeType alarmcode, ManagedObjectType managedObject,
         unsigned int managedObjectInstance, unsigned int pid);

      /**
       * Method to handle generation of informational event reports.
       */
      static void reportEvent(EventCodeType eventcode, ManagedObjectType managedObject, 
         unsigned int managedObjectInstance, unsigned int pid);

      /**
       * Return a singleton instance
       */
      static Faults* getInstance();

      /**
       * Perform initialization of shared memory queue, etc.
       * @returns OK on success; otherwise ERROR
       */
      int initialize();

   protected:

   private:

      /**
       * Constructor
       */
      Faults();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      Faults(const Faults& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      Faults& operator= (const Faults& rhs);

      /**
       * Return a pointer to the Shared Memory queue
       */
      FaultSMQueue* getQueue();

      /** Singleton instance */
      static Faults* faultInstance_;

      /** ACE Process Semaphore used to signal between enqueue/dequeue threads/processes
          when the queue is empty and non-empty */
      static ACE_Process_Semaphore* processSemaphore_;

      /** Shared Memory Queue for sending alarms, clear alarms and event reports to the Fault Manager */
      FaultSMQueue* faultSMQueue_;

};

#endif

