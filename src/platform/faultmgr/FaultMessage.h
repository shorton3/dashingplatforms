/******************************************************************************
* 
* File name:   FaultMessage.h 
* Subsystem:   Platform Services 
* Description: Fault Message class encapsulates the information for a single
*              alarm so that it can be enqueued by the originating application
*              and then dequeued by the Fault Manager process for transfer to the
*              EMS.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_FAULT_MESSAGE_H_
#define _PLAT_FAULT_MESSAGE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Based_Pointer_T.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/common/AlarmsEvents.h"

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
 * Fault Message class encapsulates the information for a single
 * alarm so that it can be enqueued by the originating application
 * and then dequeued by the Fault Manager process for transfer to the
 * EMS.
 * <p>
 * The Alarm severity and description/resolution fields are stored in the database
 * so that they can be managed via the EMS (and not hardcoded). This allows us to 
 * support severity re-assignment as well as acknowledgement.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Persistent Shared Memory backing store file that gets mapped into shared memory */
#define FAULTSM_BACKINGSTORE       "/tmp/backingstore.faultmgr"
/** Shared Memory Initialization parameters */
#define FAULTSM_QUEUENAME          "FaultSMQueue"
#define FAULTSM_QUEUEMUTEXNAME     "FaultSMQueueMutex"
#define FAULTQUEUE_SEMAPHORE_NAME  "FaultQueueSemaphore"
#define NEID_LENGTH 10
#define FAULT_BUFFER_SIZE 1024

struct FaultMessage
{
   /** NEID (network element Id) that the Fault applies to. 9 character string */
   char neid[NEID_LENGTH];
   /** Position Independent pointer to the NEID */
   ACE_Based_Pointer_Basic<char> neidPI;
   /** Managed Object type (MEMORY, COMMUNICATIONS LINK, HARD DISK, etc.) */
   ManagedObjectType managedObject;
   /** Managed Object instance (LINK 12, LINK 13, etc.) */
   unsigned int managedObjectInstance;
   /** Alarm code that applies to the fault condition */
   AlarmCodeType alarmCode;
   /** Event Report code if the fault message is an event report instead */
   EventCodeType eventCode;
   /** Severity. This field is used for event reports and clear alarms, but for raising
       alarms, this severity will be re-assigned/overriden by the severity in the database
       since we need to support severity reassignment by the user */
   AlarmSeverityType alarmSeverity;
   /** OS Process ID originating the alarm */
   unsigned int pid;
   /** Time stamp for alarm origination */
   unsigned int timeStamp;

   /** Default Constructor */
   FaultMessage();

   /** Copy Constructor */
   FaultMessage( const FaultMessage& rhs );

   /** Destructor */
   ~FaultMessage(void);

   /** Reset data members */
   void reset(void);

   /** Overloaded assignment operator */
   FaultMessage& operator= ( const FaultMessage& rhs );

   /** Overloaded equality operator */
   bool operator== (const FaultMessage& rhs) const;

   /** Overloaded comparison operator */
   bool operator< (const FaultMessage& rhs) const;

};//end FaultMessage structure

#endif
