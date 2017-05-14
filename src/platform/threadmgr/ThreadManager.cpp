/******************************************************************************
*
* File name:   ThreadManager.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <iostream>
#include <sstream>

#include <ace/Thread.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ThreadManager.h"

#include "platform/logger/Logger.h"

#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Number of times that we try to re-invoke the thread function before restarting
// the entire thread (memory space and all)
#define THREAD_FUNC_RETRY 5

// Period in which the thread monitor wakes up and checks to see if there are
// threads waiting to be restarted (in seconds)
#define THREAD_MONITOR_PERIOD 2

// Thread Manager thread monitor group Id (make sure to distinguish it from other
// threads in groups -- so it is not controlled by mistake)
#define MONITOR_GROUP_ID 17239115

// Initialize the static restart map (list of threads waiting on restart)
ThreadManager::ThreadsWaitingForRestartMap ThreadManager::restartMap_;

// Non-recursive Thread mutex to control access to the map
ACE_Thread_Mutex ThreadManager::restartMapMutex_;

// Flag to show if thread monitoring has been started yet
bool ThreadManager::isInitialized_ = false;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ACE_thread_t ThreadManager::createThread(ACE_THR_FUNC function,
                                  void *functionArgs,
                                  const char* threadName,
                                  bool shouldRestart,
                                  long threadFlags,
                                  long priority,
                                  int threadGroup,
                                  size_t stackSize)
{
   ACE_thread_t threadId = 0;

   // Perform lazy initialization of the Thread Monitoring mechanism (thread)
   if (!isInitialized_)
   {
      // First set the initialized flag
      isInitialized_ = true;
      // Recursive call to createThread
      ThreadManager::createThread((ACE_THR_FUNC)ThreadManager::startThreadMonitor, 0, "ThreadManagerMonitor", true, 
         THR_NEW_LWP | THR_JOINABLE | THR_SCHED_DEFAULT, ACE_DEFAULT_THREAD_PRIORITY, MONITOR_GROUP_ID);
   }//end if

   ostringstream ostr;
   ostr << "Received request to create thread (" << threadName << ")" << ends;
   STRACELOG(DEVELOPERLOG, THREADLOG, ostr.str().c_str());

   // Instantiate the thread parameters structure with the members (for passing to runThread).
   // Note that parameters are created on the HEAP
   ThreadParameters* threadParameters = new ThreadParameters(threadName, shouldRestart,
      function, functionArgs, threadFlags, priority, threadGroup, stackSize);

   // Spawn the thread with our standard 'Thread Hook' (runThread) and pass in the parameters. We need to use 
   // this instead of ACE's ACE_Thread_Hook so that we can 'name' the threads, etc.
   if (ACE_Thread_Manager::instance()->spawn( (ACE_THR_FUNC)ThreadManager::runThread, (void*)threadParameters, threadFlags,
       &threadId, 0 /*thread handle*/, priority, threadGroup, 0 /*stack*/, stackSize) == ERROR)
   {
      ostringstream ostr;
      ostr << "Unable to spawn thread (" << threadName << ")" << ends;
      STRACELOG(ERRORLOG, THREADLOG, ostr.str().c_str());
      return 0;
   }//end if
   else
   {
      ostringstream ostr;
      ostr << "Successfully spawned thread (" << threadName << ") returning Thread Id (" << threadId << ")" << ends;
      STRACELOG(DEBUGLOG, THREADLOG, ostr.str().c_str());
   }//end else 
   return threadId;
}//end createThread


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
ACE_thread_t* ThreadManager::createThreadPool(size_t Nthreads, 
                                  ACE_THR_FUNC function,
                                  void *functionArgs,
                                  const char* threadName,
                                  bool shouldRestart,
                                  long threadFlags,
                                  long priority,
                                  int threadGroup,
                                  size_t stackSize)
{
   ACE_thread_t* threadIds = new ACE_thread_t[Nthreads];
   size_t stackSizes[Nthreads];
   memset(stackSizes, stackSize, Nthreads);

   // Perform lazy initialization of the Thread Monitoring mechanism (thread)
   if (!isInitialized_)
   {
      // First set the initialized flag
      isInitialized_ = true;
      // Recursive call to createThread
      ThreadManager::createThread((ACE_THR_FUNC)ThreadManager::startThreadMonitor, 0, "ThreadManagerMonitor", true,
         THR_NEW_LWP | THR_JOINABLE | THR_SCHED_DEFAULT, ACE_DEFAULT_THREAD_PRIORITY, MONITOR_GROUP_ID);
   }//end if

   ostringstream ostr;
   ostr << "Received request to create thread pool (" << threadName << ") with (" << Nthreads << ") threads" << ends;
   STRACELOG(DEVELOPERLOG, THREADLOG, ostr.str().c_str());

   // Instantiate the thread parameters structure with the members (for passing to runThread).
   // Note that parameters are created on the HEAP
   ThreadParameters* threadParameters = new ThreadParameters(threadName, shouldRestart,
      function, functionArgs, threadFlags, priority, threadGroup, stackSize);

   // Spawn the threads with our standard 'Thread Hook' (runThread) and pass in the parameters. We need to use
   // this instead of ACE's ACE_Thread_Hook so that we can 'name' the threads, etc.
   if (ACE_Thread_Manager::instance()->spawn_n( threadIds, Nthreads, (ACE_THR_FUNC)ThreadManager::runThread,
       (void*)threadParameters, threadFlags, priority, threadGroup, 0 /*stack*/, stackSizes, 0 /*thread handles*/) == ERROR)
   {
      ostringstream ostr;
      ostr << "Unable to spawn thread pool (" << threadName << ")" << ends;
      STRACELOG(ERRORLOG, THREADLOG, ostr.str().c_str());
      return 0;
   }//end if
   else
   {
      ostringstream ostr;
      ostr << "Successfully spawned thread pool (" << threadName << ")" << ends;
      STRACELOG(DEBUGLOG, THREADLOG, ostr.str().c_str());
   }//end else
   return threadIds;
}//end createThreadPool


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
void ThreadManager::startThreadMonitor()
{
   while (1)
   {
      // if the map has some contents, restart those threads. Remember to protect
      // access to the map
      ThreadManager::restartMapMutex_.acquire();
      if (!ThreadManager::restartMap_.empty())
      {
         ThreadsWaitingForRestartMap::iterator restartMapIterator;
         ThreadsWaitingForRestartMap::iterator endIterator;
        
         restartMapIterator = ThreadManager::restartMap_.begin();
         endIterator = ThreadManager::restartMap_.end();

         // Loop through the map and restart each thread
         while (restartMapIterator != endIterator)
         {
            ThreadParameters* threadParms = (ThreadParameters*)restartMapIterator->second;
            // Restart the thread
            ACE_thread_t newThreadId = ThreadManager::createThread(threadParms->function_,
               threadParms->functionArgs_, threadParms->threadName_.c_str(), threadParms->shouldRestart_,
               threadParms->threadFlags_, threadParms->priority_, threadParms->threadGroup_,
               threadParms->stackSize_);

            ostringstream ostr;
            ostr << "Thread (" << threadParms->threadName_ << ") restarted with new Thread Id ("
                 << newThreadId << ")" << ends;
            STRACELOG(DEBUGLOG, THREADLOG, ostr.str().c_str());

            // Remove the thread parameters from the map after it has been restarted
            ThreadManager::restartMap_.erase(restartMapIterator->first);
            restartMapIterator++;
         }//end while
      }//end if
      ThreadManager::restartMapMutex_.release();
      sleep(THREAD_MONITOR_PERIOD);
   }//end while
}//end startThreadMonitor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ThreadManager::~ThreadManager()
{
   // If we are not going to restart this thread, then delete the thread parameters
   // structure. NOTE: Ownership of the ThreadManager object itself is handed-off
   // to the Thread_At_Exit, and it should/will be deleted upon thread finalization.
   if ((threadParameters_) && (!threadParameters_->shouldRestart_))
   {
      delete threadParameters_;
   }//end if
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description:
// Design:
//-----------------------------------------------------------------------------
void ThreadManager::apply(void)
{
   // Assume that if we want a restart, then thread death is an ERROR
   if (threadParameters_->shouldRestart_)
   {
      ostringstream ostr;
      ostr << "Thread exit handler caught thread death for thread (" << threadParameters_->threadName_
           << ") with Id (" << threadId_ << "). Attempting restart." << ends;
      STRACELOG(ERRORLOG, THREADLOG, ostr.str().c_str());

      // NOTE!!
      // We cannot just re-call createThread from this space, as the ACE_Thread_Manager
      // is locked (we are inside the lock), and we will need to lock it to call 'spawn',
      // and that will result in a DEADLOCK. Therefore, our strategy is to post the thread
      // details (threadparameters) to a static map, and have an independent thread
      // periodically every second or so check to see if the map contains any threads
      // that need restarting.
      // ALSO NOTE:!!
      // Upon restart, the thread Id originally made available to the application for this 
      // thread will be invalid. We can (IN THE FUTURE) allow the application developer to 
      // pass in a call back function (with unsigned long parameter) that can be 
      // called and passed the new Thread Id.

      // Create an STL pair to put in the map
      typedef pair<unsigned long, ThreadParameters*> ThreadRestartPair;
      ThreadRestartPair threadRestartPair(threadId_, threadParameters_);
      // Create an interator for checking insert status. Remember to protect access
      // to the map
      ThreadManager::restartMapMutex_.acquire();
      pair<map<unsigned long, ThreadParameters*>::iterator, bool> pairIterator =
      // Insert the thread parameters struct into the map
      ThreadManager::restartMap_.insert(threadRestartPair);
      // Check the status
      if (!pairIterator.second)
      {
         ostringstream ostr;
         ostr << "Error queuing thread (" << threadParameters_->threadName_ 
              << ") with Id (" << threadId_ << ") for restart. Thread will not be restarted." << ends;
         STRACELOG(ERRORLOG, THREADLOG, ostr.str().c_str()); 
      }//end if
      ThreadManager::restartMapMutex_.release();
   }//end if
   else
   {
      ostringstream ostr;
      ostr << "Thread exit handler caught thread death for (" << threadParameters_->threadName_
           << ") with Id (" << threadId_ << ")" << ends;
      STRACELOG(WARNINGLOG, THREADLOG, ostr.str().c_str());
   }//end else
}//end apply


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
ThreadManager::ThreadManager(ThreadParameters& threadParameters)
               : threadParameters_(&threadParameters)
{
   // resolve and store the child thread Id
   threadId_ = ACE_Thread::self();
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
ThreadManager* ThreadManager::getInstance(ThreadParameters& threadParameters)
{
   return new ThreadManager(threadParameters);
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
ACE_THR_FUNC_RETURN ThreadManager::runThread(ThreadParameters& threadParameters)
{
   // Attach the Thread exit handler to this thread - the ThreadParameter struct stays in 
   // scope and does not get deleted 'unless' the ThreadManager is deleted -and-
   // restart is 'false'
   ACE_Thread_Manager::instance()->at_exit(ThreadManager::getInstance(threadParameters));

   ostringstream ostr1;
   ostr1 << "Attached thread exit handler to thread (" << threadParameters.threadName_ 
         << ") with Id (" << ACE_Thread::self() << ")" << ends;
   STRACELOG(DEVELOPERLOG, THREADLOG, ostr1.str().c_str());

   // Invoke the developer-provided thread function (probably blocks forever)
   // Use try/catch exception handling incase someone is throwing one
   try
   {
      (*threadParameters.function_) (threadParameters.functionArgs_);
   }//end try
   catch (exception exc)
   {
      ostringstream ostr;
      ostr << "Caught Application Level Exception - " << exc.what() << ends;
      STRACELOG(ERRORLOG, THREADLOG, ostr.str().c_str());
   }//end catch

   // Here we loop and just manually re-invoke the thread function upto THREAD_FUNC_RETRY
   // times; after that, we exit and try to retry the whole thread space
   if (threadParameters.shouldRestart_)
   {
      int counter = 0;
      while (counter < THREAD_FUNC_RETRY)
      {
         // The thread function unexpectedly returned
         ostringstream ostr2;
         ostr2 << "Thread function (" << threadParameters.threadName_ << ") with Id (" 
               << ACE_Thread::self() << ") returned. Calling it again (count=" << counter << ")." << ends;
         STRACELOG(ERRORLOG, THREADLOG, ostr2.str().c_str());
         // Invoke the developer's function again. Use try/catch exception handling
         // incase someone is throwing one.
         try
         {
            (*threadParameters.function_) (threadParameters.functionArgs_); 
         }//end try
         catch (exception exc)
         {
            ostringstream ostr;
            ostr << "Caught Application Level Exception - " << exc.what() << ends;
            STRACELOG(ERRORLOG, THREADLOG, ostr.str().c_str());
         }//end catch
         // Increment the counter
         counter++;
      }//end while
      // Give up trying to restart the function and let the whole thread be restarted
      ostringstream ostr2;
      ostr2 << "Thread function (" << threadParameters.threadName_ << ") with Id ("
            << ACE_Thread::self() << ") returned " << counter 
            << " times. Giving up to let the thread get restarted" << ends;
      STRACELOG(ERRORLOG, THREADLOG, ostr2.str().c_str());
   }//end if
   else
   {
      // The thread function unexpectedly(?) returned, so give a warning
      ostringstream ostr2;
      ostr2 << "Thread function (" << threadParameters.threadName_ << ") with Id ("
            << ACE_Thread::self() << ") returned." << ends;
      STRACELOG(WARNINGLOG, THREADLOG, ostr2.str().c_str());
   }//end else
   return 0; 
}//end runThread


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

