/******************************************************************************
* 
* File name:   FaultSMQueue.h 
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

#ifndef _PLAT_FAULT_SM_QUEUE_H_
#define _PLAT_FAULT_SM_QUEUE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Process_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "FaultMessage.h"

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
 * FaultSMQueue sets up an Unbounded Queue in Shared Memory for the
 * purpose of raising and clearing FaultMessageType messages (alarms,
 * clear alarms, and informational event reports) to the Fault Manager
 * which will raise those to the EMS.
 * <p>
 * This shared memory queue uses the Position Independent Malloc/Allocation
 * factory in ACE, and it handles queue growth using the automatic OS
 * exception handling facilities to trap SIGSEGV and perform an ACE::remap.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class FaultSMQueue
{
   public:

      /** Shared Memory Queue type for FaultMessages */
      typedef UnboundedSMQueue<FaultMessage> FAULTSMQUEUE;

      /** Constructor */
      FaultSMQueue(const char* queueName, const char* coordinatingMutexName);

      /** Virtual Destructor */
      virtual ~FaultSMQueue();

      /** 
       * Create the (or get a reference to an already created) queue
       * @returns OK on success; otherwise ERROR
       */
      int setupQueue();

      /**
       * Enqueue a FaultMessage to the shared memory queue 
       * @returns OK on success; otherwise ERROR
       */
      int enqueueAlarm(FaultMessage& message);

      /**
       * Dequeue a FaultMessage from the shared memory queue.
       * The calling code is responsible for allocating the FaultMessage and passing it
       * in as a reference to be populated.
       * @returns OK on success; otherwise ERROR
       */
      int dequeueAlarm(FaultMessage& message);

      /** Clears the contents (alarms) of the shared memory queue */
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
      FaultSMQueue(const FaultSMQueue& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      FaultSMQueue& operator= (const FaultSMQueue& rhs);

      /** ACE Process Mutex for guarding multi-process shared memory access to the queue */
      ACE_Process_Mutex coordinatingMutex_;

      /** Name used for unique identification of the queue in Shared Memory */
      string queueName_;

      /** Shared memory allocator */
      ALLOCATOR* shmemAllocator_;

      /** Pointer to the actual shared memory queue */
      FAULTSMQUEUE* queue_;
};

#endif
