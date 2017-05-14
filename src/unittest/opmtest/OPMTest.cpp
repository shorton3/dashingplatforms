/******************************************************************************
*
* File name:   OPMTest.cpp
* Subsystem:   Platform Services
* Description: Implements the unit test code to exercise the OPM.
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
#include <sys/resource.h>
#include <ace/Task.h>
#include <ace/Thread_Manager.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OPMTest.h"

#include "platform/opm/OPM.h"

// Log Manager related includes.
#include "platform/logger/Logger.h"

// Common defines
#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)OPM Test"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------

/* For Testing */
int opmTestPoolID = 0;

#define OPM_TEST_MAX_SECONDS 5

//-----------------------------------------------------------------------------
// Method Type: Test function that gets random seconds from 1 to OPM_TEST_MAX_SECONDS
// Description: 
// Design:     
//-----------------------------------------------------------------------------
int opmTestGetRandomSeconds()
{
   return (rand() / (RAND_MAX / OPM_TEST_MAX_SECONDS) + 1);
}//end opmTestGetRandomSeconds


//-----------------------------------------------------------------------------
// Method Type: Test function that performs check-in and check-out stuff
// Description: Parameter determines if it should Loop performing check-in/out 
// Design:     
//-----------------------------------------------------------------------------
int opmTestDoCheckOutCheckIn(int shouldLoop)
{
   while (1)
   {
      //Reserve some objects from the pool
      OPMTest* object1 = (OPMTest*)OPM_RESERVE(opmTestPoolID);
      OPMTest* object2 = (OPMTest*)OPM_RESERVE(opmTestPoolID);
      OPMTest* object3 = (OPMTest*)OPM_RESERVE(opmTestPoolID);
  
      if ((object1 == NULL) || (object2 == NULL) || (object3 == NULL))
      {
         TRACELOG(ERRORLOG, OPMLOG, "OPMTest - received one or more null objects from the pool",0,0,0,0,0,0);
         return ERROR;
      }//end if

      //Do something with the objects
      object1->doSomething();
      object2->doSomething();
      object3->doSomething();

      //Release the objects back into the pool
      bool result = OPM_RELEASE((OPMBase*)object1);
           result |= OPM_RELEASE((OPMBase*)object2);
           result |= OPM_RELEASE((OPMBase*)object3);
      if (result)
      {
         TRACELOG(DEBUGLOG, OPMLOG, "OPMTest - successfully released back into the pool, Process (%d)", getpid(),0,0,0,0,0);
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, OPMLOG, "OPMTest - error releasing back into the pool, Process (%d)", getpid(),0,0,0,0,0);
      }//end else

      //If we are not looping and this is the first time through, then break out
      if (shouldLoop == FALSE)
         break;
      //Otherwise, delay a random amount of time before repeating the procedure
      sleep(2);
   }//end while
   return OK;
}//end opmTestDoCheckOutCheckIn



//-----------------------------------------------------------------------------
// Method Type: Test entry function
// Description: 
// Design:     
//-----------------------------------------------------------------------------
void opmTestStart(bool isTaskSafe)
{
   TRACELOG(DEBUGLOG, OPMLOG, "Inside opmTestStart",0,0,0,0,0,0);
   if (!isTaskSafe)
   {
      //Initialize the OPM
      OPM::initialize();

      //Create a pool of Test objects (Task unsafe)
      opmTestPoolID = OPM::createPool("OPMTestUnSafe", 0, (OPM_INIT_PTR)&OPMTest::initialize, 0.8, 
         50, 100, false, OPM_GROW_AND_SHRINK);
   }//end if
   else if (isTaskSafe)
   {
      //Initialize the OPM
      OPM::initialize();

      //Create a pool of Test objects (Task safe)
      opmTestPoolID = OPM::createPool("OPMTestSafe", 0, (OPM_INIT_PTR)&OPMTest::initialize, 0.8, 
         50, 100, true, OPM_GROW_AND_SHRINK);
   }//end else
   else
   {
      //Print usage
      printf("Usage: opmTestStart <true or false>\n");
      return;
   }//end else

   //Perform some basic check-in and check-out
   opmTestDoCheckOutCheckIn(FALSE);
}//end opmTestStart


//-----------------------------------------------------------------------------
// Method Type: Test2 entry function
// Description: Assumes that the opmTestStart function has already been called
//              and OPM has been initialized and the OPMTest pool is created.
// Design:      Accepts number of tasks to spawn for testing
//-----------------------------------------------------------------------------
void opmTest2Start(int numberOfTasks)
{
   //int taskPriority = 100;
   //int taskOptions = 0;
   //int taskStackSize = 5000;

   //Check usage
   if (numberOfTasks <= 0)
   {
      printf("Usage: opmTest2Start <numberOfTasks>\n");
      return;
   }//end if

   //create a Function pointer
   int (*funcPtr)(int);
   funcPtr = &opmTestDoCheckOutCheckIn;
 
   for (int i = 0; i < numberOfTasks; i++)
   {
      //spawn a task for performing check-in and check-out stuff
      if (ACE_Thread_Manager::instance()->spawn( (ACE_THR_FUNC) opmTestDoCheckOutCheckIn,
                                                 (void*) 0,
                                                 THR_NEW_LWP) == -1)
      {
         TRACELOG(ERRORLOG, OPMLOG, "Unable to spawn thread", 0,0,0,0,0,0);
         break;
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, OPMLOG, "opmTest2Start: Spawning test task number %d", i,0,0,0,0,0);
      }//end else
   }//end for
}//end opmTest2Start


//-----------------------------------------------------------------------------
// Method Type: Overriden initialize method
// Description: 
// Design:     
//-----------------------------------------------------------------------------
OPMBase* OPMTest::initialize(int initializer)
{
   // do some dummy stuff on bufPtr to prevent compiler warning - this code will be
   // removed by the optimizer when it runs
   int tmp __attribute__ ((unused)) = initializer;

   TRACELOG(DEBUGLOG, OPMLOG, "OPMTest initialize is called in Process (%d)", getpid(),0,0,0,0,0);
   return (new OPMTest());
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: Overriden clean method
// Description: 
// Design:     
//-----------------------------------------------------------------------------
void OPMTest::clean()
{
   TRACELOG(DEBUGLOG, OPMLOG, "OPMTest clean is called in Process (%d)", getpid(),0,0,0,0,0);
}//end clean


//-----------------------------------------------------------------------------
// Method Type: Test method to perform some action
// Description: 
// Design:     
//-----------------------------------------------------------------------------
void OPMTest::doSomething()
{
   TRACELOG(DEBUGLOG, OPMLOG, "OPMTest doing something in Process (%d)", getpid(),0,0,0,0,0);

   //Delay a random amount of time before returning
   sleep(2);
}//end doSomething


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

   // Do a simple shared memory test
#if !defined (ACE_HAS_SYSV_IPC) || defined(ACE_LACKS_SYSV_SHMEM)
   printf("\nSystem has no shared memory\n");
#else
   printf("\nSystem has shared memory\n");
#endif
 
   // Turn of OS limits
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // Initialize the Logger with local-only output
   Logger::getInstance()->initialize(true);

   // Turn on OPM Subsystem log levels
   Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
 
   // Run test #1 (not thread safe)
   opmTestStart(false);

   // Run test #1 (thread safe)
   opmTestStart(true);

   // Run test #2 with the specified number of threads
   opmTest2Start(2);

}//end main
