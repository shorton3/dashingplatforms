/******************************************************************************
* 
* File name:   ThreadManager.h 
* Subsystem:   Platform Services 
* Description: Wraps the ACE Thread mechanism in order to provide Thread
*              monitoring, Thread naming, and death detection and restart
*              capabilities.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_PLATFORM_THREAD_H_
#define _PLAT_PLATFORM_THREAD_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <map>
#include <string>

#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

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
 * ThreadManager wraps the ACE Thread mechanism in order to provide Thread
 * monitoring, Thread naming, and death detection and restart capabilities. 
 * <p>
 * The main developer interface is the createThread() method, which returns
 * an OS-assigned unique thread Id. This thread id can be stored by the application
 * and used with the ACE_Thread_Manager class for suspending, resuming, waiting on,
 * or managing groups of threads (see ace/Thread_Manager.h).
 * <p>
 * In the case of createThreadPool(), a pointer to an array of ACE_Thread_t
 * Thread Ids is returned--one for each of the N threads.
 * <p>
 * In the event of a thread death/restart, we cannot just re-call createThread from
 * the space of the ACE_At_Exit handler, as the ACE_Thread_Manager is locked (we
 * are inside the lock), and we will need to lock it to call 'spawn', and that would
 * result in a DEADLOCK. Therefore, our strategy is to post the thread
 * details (threadparameters) to a static map, and have an independent thread
 * periodically every second or so check to see if the map contains any threads
 * that need restarting. Any other strategies for notification and restart of the
 * thread are very complex!
 * <p>
 * ALSO NOTE:!! Upon restart, the thread Id originally made available to the
 * application for this thread will be invalid. We can (IN THE FUTURE) allow
 * the application developer to pass in a call back function (with unsigned
 * long parameter) that can be called and passed the new Thread Id.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ThreadManager : public ACE_At_Thread_Exit
{
   public:

      /** Virtual Destructor */
      virtual ~ThreadManager();

      /**
       * Inheritied from ACE_At_Thread_Exit. Gets called upon thread death/exit.
       */
      void apply(void);

      /**
       * Main Thread creation API
       * @param function Static C++ method or C-style function to start the new thread with
       * @param functionArgs Array of arguments to pass to the function as parameters
       * @param threadName String identifier that should be unique to each thread
       * @param shouldRestart 'true' if the thread should be restarted upon exit;otherwise, it
       *     will be allowed to terminate
       * @param threadFlags define the characteristics of the thread
       * @param priority is the relative priority of the new thread (from 1 to 127). Specifying priority
       *     other than the default is Discouraged, and should only be used RARELY!
       * @param threadGroup can be provided if thread pooling or grouping is desired.
       * @param stackSize specifies the size of the stack allocated for this thread
       * @returns the unique thread Id upon success; otherwise 0 upon failure. In Linux, this is
       * an unsigned long (see /usr/include/bits/pthread_types.h), so cannot use ERROR
       */
      static ACE_thread_t createThread(ACE_THR_FUNC function, void *functionArgs, const char* threadName, 
         bool shouldRestart, long threadFlags = THR_NEW_LWP | THR_JOINABLE | THR_SCHED_DEFAULT,
         long priority = ACE_DEFAULT_THREAD_PRIORITY, int threadGroup = -1, size_t stackSize = 0);

      /**
       * Thread Pool creation API
       * @param Nthreads Number of threads to create in the pool
       * @param function Static C++ method or C-style function to start the new thread with
       * @param functionArgs Array of arguments to pass to the function as parameters
       * @param threadName String identifier that should be unique to each thread
       * @param shouldRestart 'true' if the thread should be restarted upon exit;otherwise, it
       *     will be allowed to terminate
       * @param threadFlags define the characteristics of the thread
       * @param priority is the relative priority of the new thread (from 1 to 127). Specifying priority
       *     other than the default is Discouraged, and should only be used RARELY!
       * @param threadGroup can be provided if thread pooling or grouping is desired.
       * @param stackSize specifies the size of the stack allocated for this thread
       * @returns Pointer array of unique thread Ids upon success; otherwise 0 upon failure.
       * This array of ACE_Thread_t thread ids is allocated on the heap, so ownership is 
       * deferred to the application owner. In Linux, these values are
       * unsigned longs (see /usr/include/bits/pthread_types.h), so cannot use ERROR
       */
      static ACE_thread_t* createThreadPool(size_t Nthreads, ACE_THR_FUNC function, void *functionArgs,
         const char* threadName, bool shouldRestart, long threadFlags = THR_NEW_LWP | THR_JOINABLE | THR_SCHED_DEFAULT, 
         long priority = ACE_DEFAULT_THREAD_PRIORITY, int threadGroup = -1, size_t stackSize = 0);

   protected:

   private:

      // Structure for passing arguments to the new thread
      struct ThreadParameters
      {
         /** ThreadParameters Constructor */
         ThreadParameters(const char* threadName, bool shouldRestart, ACE_THR_FUNC function, void* functionArgs,
                    long threadFlags, long priority, int threadGroup, size_t stackSize)
                  : threadName_(threadName),
                    shouldRestart_(shouldRestart),
                    function_(*function),
                    functionArgs_(functionArgs), 
                    threadFlags_(threadFlags),
                    priority_(priority),
                    threadGroup_(threadGroup),
                    stackSize_(stackSize){}
         /**
          * Unique name given to each thread.
          */
         string threadName_;
         /**
          * Boolean indicator as to whether this thread should be restarted in the event that it fails
          */
         bool shouldRestart_;
         /**
          * ACE thread function type. (void*)
          */
         ACE_THR_FUNC function_;
         /**
          * Developer-provided function args
          */
         void* functionArgs_;
         /**
          * Thread flags (from ace/OS_NS_Threads.h):
          *   THR_CANCEL_DISABLE : Do not allow thread to cancelled
          *   THR_CANCEL_ENABLE : Allow thread to be cancelled
          *   THR_CANCEL_DEFERRED : Allow for deferred cancellation only
          *   THR_CANCEL_ASYNCHRONOUS : Allow for asynchronous cancellation only
          *   THR_DETACHED : Create a detached thread
          *   THR_BOUND : Create a thread that is bound to a kernel-schedulable entity
          *   THR_NEW_LWP : Create a kernel-level thread
          *   THR_DAEMON : Create a daemon thread
          *   THR_JOINABLE : Allow the new thread to be joined with
          *   THR_SUSPENDED : Create the thread but keep the new thread in suspended state
          *   THR_SCHED_FIFO : Schedule the new thread using the FIFO policy, if available
          *   THR_SCHED_RR : Schedule the new thread using a round robin scheme, if available
          *   THR_SCHED_DEFAULT : Use whatever default scheduling scheme is available on the OS
          *   THR_INHERIT_SCHED : Thread will inherit scheduling scheme from its parent thread
          */
         long threadFlags_;
         /**
          * Priority of the child thread (Minimum is 1; Maximum in Linux is 127)
          */
         long priority_;
         /**
          * Arbitrary thread group Id assigned by the application developer
          */
         int threadGroup_;
         /**
          * Stack size to be assigned to the new thread
          */
         size_t stackSize_;
      };//end ThreadParameters

      /** Map is used to maintain a list of threads that have/are dying and must be restarted. Here,
          the key is the OS assigned unique thread Id */
      typedef map<unsigned long, ThreadParameters*, less<unsigned long> > ThreadsWaitingForRestartMap;

      /** Constructor */
      ThreadManager(ThreadParameters& threadParameters);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ThreadManager(const ThreadManager& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ThreadManager& operator= (const ThreadManager& rhs);

      /**
       * Static method to generate a new, dynamic instance of the ACE_At_Thread_Exit
       */
      static ThreadManager* getInstance(ThreadParameters& threadParameters);

      /**
       * Static method that the thread 'actually' gets invoked with.
       * Thread parameters contains: string& threadName, bool shouldRestart, ACE_THR_FUNC function,
       *    void *functionArgs
       */
      static ACE_THR_FUNC_RETURN runThread(ThreadParameters& threadParameters);

      /**
       * Static method lazily invoked to start the thread monitoring mechanism that checks
       * the restart map.
       */
      static void startThreadMonitor();

      /**
       * Static instance of the restart map. Upon receiving an OS Signal, the signal handler will
       * read all of the ThreadParameter structs from the map and restart them.
       */
      static ThreadsWaitingForRestartMap restartMap_;

      /** Non-recursive Thread Mutex for controlling access to the map */
      static ACE_Thread_Mutex restartMapMutex_;

      /** Flag that indicates if the lazy initialization of the thread monitor has occurred yet */
      static bool isInitialized_;

      /** Thread Parameters structure member variable */
      ThreadParameters* threadParameters_;

      /**
       * OS assigned unique thread Id for the child thread
       */
      ACE_thread_t threadId_;

};

#endif
