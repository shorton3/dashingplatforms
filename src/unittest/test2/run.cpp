#include "run.h"
#include <ace/Thread_Mutex.h>
#include <iostream>
using namespace std;

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Test 2"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// implementation of instance method:
bool run::someInstanceMethod(void)
{
   ACE_Thread_Mutex threadMutex;

   while (1)
   { 
      threadMutex.acquire();

      cout << "Acquired thread mutex" << endl;

      threadMutex.release();
   }//end while

   return true;
}//end someInstanceMethod


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

   run* runInst = new run();

   runInst->someInstanceMethod();

   return 0;
}//end main
