/******************************************************************************
*
* File name:   ThreadTest.cpp
* Subsystem:   Platform Services
* Description: Unit Tests for Thread Monitoring, Recovery, and Restart
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

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/resource.h>

#include <ace/OS_NS_unistd.h>
#include <ace/Thread_Manager.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ThreadTest.h"

#include "platform/logger/Logger.h"

#include "platform/common/Defines.h"

#include "platform/threadmgr/ThreadManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Thread Test"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ThreadTest::ThreadTest()
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ThreadTest::~ThreadTest()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialization
// Design:
//-----------------------------------------------------------------------------
void ThreadTest::initialize()
{
   // Here we assign a thread Group Id, so we can use ACE_Thread_Manager to wait on that group id
   ACE_thread_t threadId = ThreadManager::createThread((ACE_THR_FUNC)ThreadTest::performTests, 0, "THREADTEST", false,
      THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED/*Thread Characteristics*/, ACE_DEFAULT_THREAD_PRIORITY /*priority*/,
      1 /*Arbitrary Thread Group Id*/);

   TRACELOG(DEBUGLOG, THREADLOG, "Spawned ThreadTest thread with id %ul", threadId,0,0,0,0,0);

   // Now wait on the first thread to complete
   if (ACE_Thread_Manager::instance()->wait_grp(1) == ERROR)
   {
      TRACELOG(ERRORLOG, THREADLOG, "Waiting on thread group to complete failed",0,0,0,0,0,0);
   }//end if

   TRACELOG(DEBUGLOG, THREADLOG, "Preparing to Spawn Restart Thread test",0,0,0,0,0,0);

   threadId = ThreadManager::createThread((ACE_THR_FUNC)ThreadTest::performRestartTests, 0, "RESTARTTHREAD", true);
   
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Perform unit tests 
// Design:
//-----------------------------------------------------------------------------
void ThreadTest::performTests(void)
{
   TRACELOG(DEBUGLOG, THREADLOG, "Beginning test for spawning non-restarting thread",0,0,0,0,0,0);

   int i;
   for (i = 0; i < 5; i++)
   {
      TRACELOG(DEBUGLOG, THREADLOG, "Performing Thread Test, iteration %d", i,0,0,0,0,0);
      sleep(2);
   }//end for
}//end performTests


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Perform unit tests for restarting threads
// Design:
//-----------------------------------------------------------------------------
void ThreadTest::performRestartTests(void)
{
   TRACELOG(DEBUGLOG, THREADLOG, "Beginning test for spawning restarting thread",0,0,0,0,0,0);

   int i;
   for (i = 0; i < 5; i++)
   {
      TRACELOG(DEBUGLOG, THREADLOG, "Performing Thread Restarting Test, iteration %d", i,0,0,0,0,0);
      sleep(2);
   }//end for
}//end performRestartTests


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function Type: main function for test binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   // do some dummy stuff on bufPtr to prevent compiler warning - this code will be
   // removed by the optimizer when it runs
   int tmpInt __attribute__ ((unused)) = argc;
   char** tmpChar __attribute__ ((unused)) = argv;

   // Turn of OS limits
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // Initialize the Logger; Set the logger output to be local stdout/stderr
   Logger::getInstance()->initialize(true);

   // Turn on All relevant Subsystem logging levels
   Logger::setSubsystemLogLevel(THREADLOG, DEVELOPERLOG);

   ThreadTest* threadTest = new ThreadTest();
   if (!threadTest)
   {
      TRACELOG(ERRORLOG, THREADLOG, "Could not create Thread Test instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   threadTest->initialize();

   TRACELOG(DEBUGLOG, THREADLOG, "Thread Test initialize returned",0,0,0,0,0,0);

   // Use Thread_Manager to wait on all child threads, no matter what they are 
   // associated with
   ACE_Thread_Manager::instance()->wait(); 

}//end main

