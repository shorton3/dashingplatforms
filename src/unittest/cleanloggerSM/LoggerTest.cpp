/******************************************************************************
*
* File name:   LoggerTest.cpp
* Subsystem:   Platform Services
* Description: Unit Test to clear the contents of shared memory 
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
#include <string>
#include <sstream>
#include <iostream>
#include <sys/resource.h>

#include <ace/OS_NS_unistd.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LoggerTest.h"

#include "platform/logger/Logger.h"
#include "platform/logger/LoggerCommon.h"
#include "platform/logger/LoggerSMConfig.h"
#include "platform/logger/LoggerSMConfigValues.h"
#include "platform/logger/LoggerSMQueue.h"

#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Clean Logger SM Utility"
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
LoggerTest::LoggerTest()
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LoggerTest::~LoggerTest()
{
}//end virtual destructor


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

   // Directly create the Logger shared memory queue
   LoggerSMQueue loggerSMQueue(LOGSM_QUEUENAME, LOGSM_QUEUEMUTEXNAME);
   // Find the Queue
   loggerSMQueue.setupQueue();
   // Clear the Contents
   cout << "Clearing out the contents of the Logger SM Queue..." << endl;
   loggerSMQueue.clearQueue();

   // Directly create the Logger shared memory config
   LoggerSMConfig loggerSMConfig(LOG_CONFIG_NAME, LOG_CONFIG_MUTEXNAME);
   // Initialize the shared memory
   loggerSMConfig.initialize();
   // Reset the configuration values structure
   cout << "Resetting Logger SM Config values to defaults..." << endl;
   loggerSMConfig.reset();

}//end main

