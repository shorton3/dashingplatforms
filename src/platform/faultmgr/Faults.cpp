/******************************************************************************
*
* File name:   Faults.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Process_Semaphore.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "Faults.h"
#include "FaultMessage.h"
#include "FaultSMQueue.h"

#include "platform/common/Defines.h"
#include "platform/common/MailboxNames.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Singleton instance variable
Faults* Faults::faultInstance_ = NULL;

ACE_Process_Semaphore* Faults::processSemaphore_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
Faults::Faults()
{
   faultSMQueue_ = new FaultSMQueue(FAULTSM_QUEUENAME, FAULTSM_QUEUEMUTEXNAME);

   // Create/map the process semaphore. Initialize it to be blocked
   processSemaphore_ = new ACE_Process_Semaphore(0, FAULTQUEUE_SEMAPHORE_NAME);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
Faults::~Faults()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return a singleton instance
// Design:
//-----------------------------------------------------------------------------
Faults* Faults::getInstance()
{ 
   if (faultInstance_ == NULL)
   { 
      faultInstance_ = new Faults();
   }//end if
   return faultInstance_;
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to handle enqueuing/raising of the alarms
// Design: 
//-----------------------------------------------------------------------------
void Faults::raiseAlarm(const char* neid, AlarmCodeType alarmcode, ManagedObjectType managedObject, 
   unsigned int managedObjectInstance, unsigned int pid)
{
   FaultMessage faultMessage;

   // Make sure we set the position independent pointer so we can file the memory on the other side
   ACE_OS::strncpy(faultMessage.neid, neid, 10);
   faultMessage.neidPI = faultMessage.neid;

   faultMessage.managedObject = managedObject;
   faultMessage.managedObjectInstance = managedObjectInstance;
   faultMessage.alarmCode = alarmcode;
   // Severity will be re-assigned from the database value on the ems (support for severity re-assignment)
   faultMessage.alarmSeverity = ALARM_INDETERMINATE;
   faultMessage.pid = pid;
   // Put on the current timestamp
   faultMessage.timeStamp = time(0);

   // Post the faultMessage into the message queue for Fault Manager to process
   if(faultInstance_->getQueue()->enqueueAlarm(faultMessage) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error enqueuing alarm into shared memory",0,0,0,0,0,0);
   }//end if

   // Release the Blocking Process Semaphore to wake-up the dequeue thread and increment the semaphore
   processSemaphore_->release();
}//end raiseAlarm


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to handle enqueuing/clearing of the alarms
// Design:
//-----------------------------------------------------------------------------
void Faults::clearAlarm(const char* neid, AlarmCodeType alarmcode, ManagedObjectType managedObject, 
   unsigned int managedObjectInstance, unsigned int pid)
{
   FaultMessage faultMessage;

   // Make sure we set the position independent pointer so we can file the memory on the other side
   ACE_OS::strncpy(faultMessage.neid, neid, 10);
   faultMessage.neidPI = faultMessage.neid;

   faultMessage.managedObject = managedObject;
   faultMessage.managedObjectInstance = managedObjectInstance;
   faultMessage.alarmCode = alarmcode;
   // Severity of 'clear' will not be re-assigned, this is how we distinguish between
   // alarms, clears, and event reports on the Fault Manager side of the queue
   faultMessage.alarmSeverity = ALARM_CLEAR;
   faultMessage.pid = pid;
   // Put on the current timestamp
   faultMessage.timeStamp = time(0);

   // Post the faultMessage into the message queue for Fault Manager to process
   if(faultInstance_->getQueue()->enqueueAlarm(faultMessage) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error enqueuing clear alarm into shared memory",0,0,0,0,0,0);
   }//end if

   // Release the Blocking Process Semaphore to wake-up the dequeue thread and increment the semaphore
   processSemaphore_->release();
}//end clearAlarm


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to handle generation of informational event reports
// Design:
//-----------------------------------------------------------------------------
void Faults::reportEvent(EventCodeType eventcode, ManagedObjectType managedObject, 
   unsigned int managedObjectInstance, unsigned int pid)
{
   FaultMessage faultMessage;

   faultMessage.managedObject = managedObject;
   faultMessage.managedObjectInstance = managedObjectInstance;
   faultMessage.eventCode = eventcode;
   // Severity of 'informational event' will not be re-assigned, this is how we distinguish between
   // alarms, clears, and event reports on the Fault Manager side of the queue
   faultMessage.alarmSeverity = INFORMATIONAL_EVENT;
   faultMessage.pid = pid;
   // Put on the current timestamp
   faultMessage.timeStamp = time(0);

   // Post the faultMessage into the message queue for Fault Manager to process
   if(faultInstance_->getQueue()->enqueueAlarm(faultMessage) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error enqueuing event report into shared memory",0,0,0,0,0,0);
   }//end if

   // Release the Blocking Process Semaphore to wake-up the dequeue thread and increment the semaphore
   processSemaphore_->release();
}//end reportEvent


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform initialization of shared memory queue, etc.
// Design:
//-----------------------------------------------------------------------------
int Faults::initialize()
{
   // setup the Shared Memory Queue for alarms
   if (faultSMQueue_->setupQueue() == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Unable to setup Faults queue",0,0,0,0,0,0);
      return ERROR;
   }//end if

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return a pointer to the Shared Memory queue
// Design:
//-----------------------------------------------------------------------------
FaultSMQueue* Faults::getQueue()
{
   return faultSMQueue_;
}//end getQueue

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

