/******************************************************************************
*
* File name:   LoggerSMQueue.cpp
* Subsystem:   Platform Services
* Description: This class sets up an Unbounded Queue in Shared Memory for the
*              purpose of exchanging LoggerMessageType messages between processes
*              and the LoggerProcessor which controls the output flow of logs
*              to log files.
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

#include "LoggerSMQueue.h"

#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Node<LogMessage>;
template class ACE_Unbounded_Queue<LogMessage>;
template class ACE_Unbounded_Queue_Iterator<LogMessage>;
template class UnboundedSMQueue<LogMessage>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Node<LogMessage>
#pragma instantiate ACE_Unbounded_Queue<LogMessage>
#pragma instantiate ACE_Unbounded_Queue_Iterator<LogMessage>
#pragma instantiate UnboundedSMQueue<LogMessage>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LoggerSMQueue::LoggerSMQueue(const char* queueName, const char* coordinatingMutexName)
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
LoggerSMQueue::~LoggerSMQueue()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Create the (or get a reference to an already created) queue
// Design:
//-----------------------------------------------------------------------------
int LoggerSMQueue::setupQueue()
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
      cout << "Logger SM Queue: found already existing SM Queue" << endl;
      queue_ = (LOGGERSMQUEUE*) queue;
      return OK;
   }//end if

   // Create a new map (because we've just created a new memory-mapped file).
   size_t queue_size = sizeof (LOGGERSMQUEUE);
   queue = shmemAllocator_->malloc (queue_size);

   // If allocation failed ...
   if (queue == 0)
   {
      cout << "Logger SM Queue: allocation for the queue size failed. Run as 'root' in order to allocate shared memory." << endl;
      return ERROR;
   }//end if

   new (queue) LOGGERSMQUEUE (shmemAllocator_);
   if (shmemAllocator_->bind (queueName_.c_str(), queue) == ERROR)
   {
      cout << "Logger SM Queue: bind to the queue failed" << endl;
      shmemAllocator_->remove();
      return ERROR;
   }//end if

   // Keep a pointer to the queue
   queue_ = (LOGGERSMQUEUE*) queue;

   return OK;
}//end setupQueue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Enqueue a LogMessage to the shared memory queue
// Design:
//-----------------------------------------------------------------------------
int LoggerSMQueue::enqueueLog(LogMessage& message)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);
   //cout << "Logger SM Queue: Enqueuing LogMessage to shared memory" << endl;

   if (queue_->enqueue_tail(message, shmemAllocator_) == ERROR)
   {
      cout << "Logger SM Queue: Enqueue to shared memory queue failed" << endl;
      return ERROR;
   }//end if
   return OK; 
}//end enqueueLog


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Dequeue a LogMessage message from the shared memory queue
// Design:
//-----------------------------------------------------------------------------
int LoggerSMQueue::dequeueLog(LogMessage& message)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);

   // Check for anything to process
   if (queue_->is_empty (shmemAllocator_))
   {
      return ERROR;
   }//end if
                                                                                                       
   if (queue_->dequeue_head(message, shmemAllocator_) == ERROR)
   {
      cout << "Logger SM Queue: Error dequeuing from queue" << endl;
      return ERROR;
   }//end if

   return OK;
}//end dequeueLog


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns true if the queue is empty
// Design:
//-----------------------------------------------------------------------------
bool LoggerSMQueue::isEmpty()
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, true);
   if (queue_->is_empty(shmemAllocator_))
      return true;
   //else 
   return false;
}//end isEmpty


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Clear all queue contents (logs) from shared memory
// Design:
//-----------------------------------------------------------------------------
void LoggerSMQueue::clearQueue()
{
   ACE_GUARD (ACE_Process_Mutex, ace_mon, coordinatingMutex_);
   queue_->delete_nodes(shmemAllocator_);
}//end clearQueue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string LoggerSMQueue::toString()
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

