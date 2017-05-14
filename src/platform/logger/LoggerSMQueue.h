/******************************************************************************
* 
* File name:   LoggerSMQueue.h 
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

#ifndef _PLAT_LOGGER_SM_QUEUE_H_
#define _PLAT_LOGGER_SM_QUEUE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Process_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LogMessage.h"

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
 * LoggerSMQueue sets up an Unbounded Queue in Shared Memory for the
 * purpose of exchanging LoggerMessageType messages between processes
 * and the LoggerProcessor which controls the output flow of logs
 * to log files.
 * <p>
 * This shared memory queue uses the Position Independent Malloc/Allocation
 * factory in ACE, and it handles queue growth using the automatic OS
 * exception handling facilities to trap SIGSEGV and perform an ACE::remap.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class LoggerSMQueue
{
   public:

      /** Shared Memory Queue type for LogMessages */
      typedef UnboundedSMQueue<LogMessage> LOGGERSMQUEUE;

      /** Constructor */
      LoggerSMQueue(const char* queueName, const char* coordinatingMutexName);

      /** Virtual Destructor */
      virtual ~LoggerSMQueue();

      /** 
       * Create the (or get a reference to an already created) queue
       * @returns OK on success; otherwise ERROR
       */
      int setupQueue();

      /**
       * Enqueue a LogMessage to the shared memory queue 
       * @returns OK on success; otherwise ERROR
       */
      int enqueueLog(LogMessage& message);

      /**
       * Dequeue a LogMessage from the shared memory queue.
       * The calling code is responsible for allocating the LogMessage and passing it
       * in as a reference to be populated.
       * @returns OK on success; otherwise ERROR
       */
      int dequeueLog(LogMessage& message);

      /** Clears the contents (logs) of the shared memory queue */
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
      LoggerSMQueue(const LoggerSMQueue& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LoggerSMQueue& operator= (const LoggerSMQueue& rhs);

      /** ACE Process Mutex for guarding multi-process shared memory access to the queue */
      ACE_Process_Mutex coordinatingMutex_;

      /** Name used for unique identification of the queue in Shared Memory */
      string queueName_;

      /** Shared memory allocator */
      ALLOCATOR* shmemAllocator_;

      /** Pointer to the actual shared memory queue */
      LOGGERSMQUEUE* queue_;
};

#endif
