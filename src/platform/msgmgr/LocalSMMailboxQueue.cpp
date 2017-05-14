/******************************************************************************
*
* File name:   LocalSMMailboxQueue.cpp
* Subsystem:   Platform Services
* Description: This class sets up an Unbounded Queue in Shared Memory for the
*              purpose of exchanging MessageBase Mailbox messages between processes
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

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalSMMailboxQueue.h"
#include "MessageBase.h"

#include "platform/logger/Logger.h"

#include "platform/utilities/SharedMemoryManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Node<LocalSMBuffer>;
template class ACE_Unbounded_Queue<LocalSMBuffer>;
template class ACE_Unbounded_Queue_Iterator<LocalSMBuffer>;
template class UnboundedSMQueue<LocalSMBuffer>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Node<LocalSMBuffer>
#pragma instantiate ACE_Unbounded_Queue<LocalSMBuffer>
#pragma instantiate ACE_Unbounded_Queue_Iterator<LocalSMBuffer>
#pragma instantiate UnboundedSMQueue<LocalSMBuffer>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LocalSMMailboxQueue::LocalSMMailboxQueue(const char* queueName, const char* coordinatingMutexName)
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
LocalSMMailboxQueue::~LocalSMMailboxQueue()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Create the (or get a reference to an already created) queue
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailboxQueue::setupQueue()
{
   void *queue = 0;

   // No Need to Guard this method with the coordinating Mutex ??

   // First let's get a pointer to the common shared memory allocator
   shmemAllocator_ = SharedMemoryManager::getAllocator();
   if (shmemAllocator_ == NULL)
   {
      return ERROR;
   }//end if

   // This is the easy case since if we find the queue in the
   // memory-mapped file we know it's already initialized.
   if (shmemAllocator_->find (queueName_.c_str(), queue) == 0)
   {
      TRACELOG(DEBUGLOG, MSGMGRLOG, "Found an already existing Local SM Mailbox Queue",0,0,0,0,0,0);
      queue_ = (LOCAL_SM_MAILBOX_QUEUE*) queue;
      return OK;
   }//end if

   // Create a new map (because we've just created a new memory-mapped file).
   size_t queue_size = sizeof (LOCAL_SM_MAILBOX_QUEUE);
   queue = shmemAllocator_->malloc (queue_size);

   // If allocation failed ...
   if (queue == 0)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Allocation for the queue size failed. Run as 'root' in order to allocate shared memory.",0,0,0,0,0,0);
      return ERROR;
   }//end if

   new (queue) LOCAL_SM_MAILBOX_QUEUE (shmemAllocator_);
   if (shmemAllocator_->bind (queueName_.c_str(), queue) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Bind to the queue failed",0,0,0,0,0,0);
      shmemAllocator_->remove();
      return ERROR;
   }//end if

   // Keep a pointer to the queue
   queue_ = (LOCAL_SM_MAILBOX_QUEUE*) queue;

   return OK;
}//end setupQueue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Enqueue a Message to the shared memory queue
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailboxQueue::enqueueMessage(LocalSMBuffer& buffer)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Enqueuing message to shared memory", 0,0,0,0,0,0);

   if (queue_->enqueue_tail(buffer, shmemAllocator_) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Enqueue to shared memory queue failed",0,0,0,0,0,0);
      return ERROR;
   }//end if
   return OK;
}//end enqueueLog


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Dequeue a Message message from the shared memory queue
// Design:
//-----------------------------------------------------------------------------
int LocalSMMailboxQueue::dequeueMessage(LocalSMBuffer& buffer)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);

   // Check for anything to process
   if (queue_->is_empty (shmemAllocator_))
   {
      return ERROR;
   }//end if

   if (queue_->dequeue_head(buffer, shmemAllocator_) == ERROR)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Error dequeuing from shared memory queue",0,0,0,0,0,0);
      return ERROR;
   }//end if

   return OK;
}//end dequeueLog


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns true if the queue is empty
// Design:
//-----------------------------------------------------------------------------
bool LocalSMMailboxQueue::isEmpty()
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, true);
   if (queue_->is_empty(shmemAllocator_) == 1)
      return true;
   //else
   return false;
}//end isEmpty


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Clear all queue MessageBase contents from shared memory
// Design:
//-----------------------------------------------------------------------------
void LocalSMMailboxQueue::clearQueue()
{
   ACE_GUARD (ACE_Process_Mutex, ace_mon, coordinatingMutex_);
   queue_->delete_nodes(shmemAllocator_);
}//end clearQueue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string LocalSMMailboxQueue::toString()
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

