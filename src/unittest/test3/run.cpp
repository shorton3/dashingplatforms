#include <string>
#include <iostream>

#include "run.h"

using namespace std;

#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Test 3"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;


// implementation of some static method:
bool run::someStaticMethod(void)
{
   bool boolean1 = false;
   if (boolean1)
   {
      cout << "FAILURE1" << endl;
   }//end if
   else
   {
      cout << "SUCCESS1" << endl;
   }//end else

   bool boolean2 = true;
   if (boolean2)
   {
      cout << "SUCCESS2" << endl;
   }//end if
   else
   {
      cout << "FAILURE2" << endl;
   }//end else 
   return true;
}//end someStaticMethod


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

   // Run some tests
   run::someStaticMethod();

   return 0;
}//end main
