#include "output.h"
#include "run.h"

#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Test 1"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// dummy friend function
void doFriendWork()
{
   ;
}

// implementation of instance method:
bool run::someInstanceMethod(void)
{
   DO_OUTPUT(1,2,3);
   return false;
}

// implementation of some static method:
bool run::someStaticMethod(void)
{
   DO_OUTPUT(1,2,3);
   return false;
}


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

   return 0;
}//end main
