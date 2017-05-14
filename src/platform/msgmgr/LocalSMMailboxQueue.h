/******************************************************************************
* 
* File name:   LocalSMMailboxQueue.h 
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

#ifndef _PLAT_LOCAL_SM_MAILBOX_QUEUE_H_
#define _PLAT_LOCAL_SM_MAILBOX_QUEUE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Process_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalSMBuffer.h"

#include "platform/common/Defines.h"

#include "platform/utilities/UnboundedSMQueue.h"

#include "platform/utilities/SharedMemoryManager.h"

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
 * LocalSMMailboxQueue sets up an Unbounded Queue in Shared Memory for the 
 * purpose of exchanging MessageBase Mailbox messages between processes. 
 * <p>
 * For simplicity, we depend on using the MessageBuffer class to serialize
 * and deserialize the Message into a raw buffer. This buffer is assumed
 * to be no larger than MAX_MESSAGE_LENGTH, and we wrap the raw buffer with
 * a LocalSMBuffer which provides the Position Independent semantics. It is
 * somewhat undesirable that we have to perform serialization/deserialization
 * as well as deep copy; however if we were to attempt manage our variable
 * length MessageBase messages in shared memory, it would not have been
 * possible to maintain the generic nature of the API exposed to the developer.
 * (NOTE that this means of IPC will still be faster than going through the
 * network stack).
 * <p>
 * This shared memory queue uses the Position Independent Malloc/Allocation
 * factory in ACE, and it handles queue growth using the automatic OS
 * exception handling facilities to trap SIGSEGV and perform an ACE::remap.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Shared Memory Initialization parameters */
#define LOCALSM_QUEUENAME "LocalSMMailboxQueue"

class LocalSMMailboxQueue
{
   public:

      /** Shared Memory Queue type for LocalSMBuffer Messages */
      typedef UnboundedSMQueue<LocalSMBuffer> LOCAL_SM_MAILBOX_QUEUE;

      /** Constructor */
      LocalSMMailboxQueue(const char* queueName, const char* coordinatingMutexName);

      /** Virtual Destructor */
      virtual ~LocalSMMailboxQueue();

      /**
       * Create the (or get a reference to an already created) queue
       * @returns OK on success; otherwise ERROR
       */
      int setupQueue();

      /**
       * Enqueue a Message to the shared memory queue
       * @returns OK on success; otherwise ERROR
       */
      int enqueueMessage(LocalSMBuffer& buffer);

      /**
       * Dequeue a Message from the shared memory queue.
       * The calling code is responsible for allocating the Message and passing it
       * in as a reference to be populated.
       * @returns OK on success; otherwise ERROR
       */
      int dequeueMessage(LocalSMBuffer& buffer);

      /** Clears the MessageBase contents of the shared memory queue */
      void clearQueue();

      /** Returns true if the queue is empty */
      bool isEmpty();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LocalSMMailboxQueue(const LocalSMMailboxQueue& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LocalSMMailboxQueue& operator= (const LocalSMMailboxQueue& rhs);

      /** ACE Process Mutex for guarding multi-process shared memory access to the queue */
      ACE_Process_Mutex coordinatingMutex_;

      /** Name used for unique identification of the queue in Shared Memory */
      string queueName_;

      /** Shared memory allocator */
      ALLOCATOR* shmemAllocator_;

      /** Pointer to the actual shared memory queue */
      LOCAL_SM_MAILBOX_QUEUE* queue_;

};

#endif
