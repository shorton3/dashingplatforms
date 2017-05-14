/******************************************************************************
*
* File name:   FaultSMQueue.cpp
* Subsystem:   Platform Services
* Description: This class sets up an Unbounded Queue in Shared Memory for the
*              purpose of raising and clearing FaultMessageType messages (alarms,
*              clear alarms, and informational event reports) to the Fault Manager
*              which will raise those to the EMS.
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

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "FaultSMQueue.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Node<FaultMessage>;
template class ACE_Unbounded_Queue<FaultMessage>;
template class ACE_Unbounded_Queue_Iterator<FaultMessage>;
template class UnboundedSMQueue<FaultMessage>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Node<FaultMessage>
#pragma instantiate ACE_Unbounded_Queue<FaultMessage>
#pragma instantiate ACE_Unbounded_Queue_Iterator<FaultMessage>
#pragma instantiate UnboundedSMQueue<FaultMessage>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
FaultSMQueue::FaultSMQueue(const char* queueName, const char* coordinatingMutexName)
              : coordinatingMutex_(coordinatingMutexName),
                queueName_(queueName),
                shmemAllocator_(NULL),
                queue_(NULL)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
FaultSMQueue::~FaultSMQueue()
{
   shmemAllocator_->remove();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Create the (or get a reference to an already created) queue
// Design:
//-----------------------------------------------------------------------------
int FaultSMQueue::setupQueue()
{
   void *queue = 0;

   // No Need to Guard this method with the coordinating Mutex ??

   // First let's get a pointer to the common shared memory allocator
   shmemAllocator_ = SharedMemoryManager::getAllocator();
   if (shmemAllocator_ == NULL)
   {
      return ERROR;
   }//end if

   // This is the easy case since if we find the SM queue in the
   // memory-mapped file we know it's already initialized.
   if (shmemAllocator_->find (queueName_.c_str(), queue) == 0)
   {
      TRACELOG(DEBUGLOG, FAULTMGRLOG, "Found already existing SM Queue",0,0,0,0,0,0);
      queue_ = (FAULTSMQUEUE*) queue;
      return OK;
   }//end if

   // Create a new map (because we've just created a new memory-mapped file).
   size_t queue_size = sizeof (FAULTSMQUEUE);
   queue = shmemAllocator_->malloc (queue_size);

   // If allocation failed ...
   if (queue == 0)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Allocation for the queue size failed. Run as 'root' in order to allocate shared memory.",0,0,0,0,0,0);
      return ERROR;
   }//end if

   new (queue) FAULTSMQUEUE (shmemAllocator_);
   if (shmemAllocator_->bind (queueName_.c_str(), queue) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Bind to the queue failed",0,0,0,0,0,0);
      shmemAllocator_->remove();
      return ERROR;
   }//end if

   // Keep a pointer to the queue
   queue_ = (FAULTSMQUEUE*) queue;

   return OK;
}//end setupQueue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Enqueue an Alarm to the shared memory queue
// Design:
//-----------------------------------------------------------------------------
int FaultSMQueue::enqueueAlarm(FaultMessage& message)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);
   //TRACELOG(DEBUGLOG, FAULTMGRLOG, "Enqueuing FaultMessage to shared memory",0,0,0,0,0,0);

   if (queue_->enqueue_tail(message, shmemAllocator_) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Enqueue to shared memory queue failed",0,0,0,0,0,0);
      return ERROR;
   }//end if
   return OK; 
}//end enqueueAlarm


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Dequeue an Alarm from the shared memory queue
// Design:
//-----------------------------------------------------------------------------
int FaultSMQueue::dequeueAlarm(FaultMessage& message)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);

   // Check for anything to process
   if (queue_->is_empty (shmemAllocator_))
   {
      return ERROR;
   }//end if
                                                                                                       
   if (queue_->dequeue_head(message, shmemAllocator_) == ERROR)
   {
      TRACELOG(ERRORLOG, FAULTMGRLOG, "Error dequeuing from queue",0,0,0,0,0,0);
      return ERROR;
   }//end if

   return OK;
}//end dequeueAlarm


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns true if the queue is empty
// Design:
//-----------------------------------------------------------------------------
bool FaultSMQueue::isEmpty()
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, true);
   if (queue_->is_empty(shmemAllocator_))
      return true;
   //else 
   return false;
}//end isEmpty


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Clear all queue contents (alarms) from shared memory
// Design:
//-----------------------------------------------------------------------------
void FaultSMQueue::clearQueue()
{
   ACE_GUARD (ACE_Process_Mutex, ace_mon, coordinatingMutex_);
   queue_->delete_nodes(shmemAllocator_);
}//end clearQueue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string FaultSMQueue::toString()
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

