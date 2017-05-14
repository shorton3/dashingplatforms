/******************************************************************************
*
* File name:   LoggerTest.cpp
* Subsystem:   Platform Services
* Description: Unit Tests for the Logger Functionality
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
#include <sys/resource.h>

#include <ace/OS_NS_unistd.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LoggerTest.h"

#include "platform/logger/Logger.h"

#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Logger Test"
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
// Method Type: INSTANCE
// Description: Perform unit tests 
// Design:
//-----------------------------------------------------------------------------
void LoggerTest::performTests()
{
   int i;
   for (i = 0; i < 100; i++)
   {
      TRACELOG(DEBUGLOG, OPMLOG, "Performing -LOCAL- Logger Test for TRACELOG, iteration %d", i,0,0,0,0,0);
      sleep(2);
   }//end for

   ostringstream ostr;
   string s = "iteration ";
   ostr << "Performing -LOCAL- Logger Test for STRACELOG, " << s << i++ << ends;
   STRACELOG(DEBUGLOG, OPMLOG, ostr.str().c_str());

   sleep(2);

   ostringstream ostr2;
   ostr << "Performing Another -LOCAL- Logger Test for STRACELOG, " << s << i++ << ends;
   STRACELOG(DEBUGLOG, OPMLOG, ostr.str().c_str());

   sleep(2);
}//end performTests


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
   Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);

   LoggerTest* loggerTest = new LoggerTest();
   if (!loggerTest)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Could not create Logger Test instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   while (1)
   {
      loggerTest->performTests();
   }//end while

}//end main

