/******************************************************************************
*
* File name:   DebugUtils.cpp
* Subsystem:   Platform Services
* Description: Various debugging methods for use within the system by development
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

#include <cstdio>
#include <cstdlib>

#include <ace/OS.h>

#include <execinfo.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DebugUtils.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Capture a full system stack trace and output it to stdout
// Design:     
//-----------------------------------------------------------------------------
void DebugUtils::printStackTrace()
{
   // Here we use the glibc 2.0 features of the compiler to get the stack trace
   void *array[128];
   size_t size;
   char **strings;

   printf("\n########## Backtrace ##########\n");
   size = backtrace (array, sizeof(array));
   strings = backtrace_symbols (array, size);

   printf ("Obtained %zd stack frames:\n", size);

   for (size_t i = 0; i < size; i++)
   {
      printf ("%s\n", strings[i]);
   }//end for

   free (strings);
   printf("\n######## Backtrace Done #######\n");
}//end printStackTrace


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Capture a full system stack trace and output it to a file
// Design:
//-----------------------------------------------------------------------------
void DebugUtils::printStackTraceToFile()
{
   // Here we use the glibc 2.0 features of the compiler to get the stack trace
   void *array[128];
   size_t size;

   // Open a file for writing/appending the stack trace to
   FILE *fp = ACE_OS::fopen ("/tmp/platform.stackTrace", ACE_TEXT ("a"));
   if (fp == 0)
   {
      printf("\nCannot open file for printStackTraceToFile\n");
      return;
   }//end if

   ACE_OS::fputs("\n########## Backtrace ##########\n",fp);

   size = backtrace (array, sizeof(array));

   ACE_OS::fprintf(fp, "Obtained %zd stack frames:\n", size);

   // Suspect that this uses unbuffered IO since it comes out before the header...
   backtrace_symbols_fd (array, size, fp->_fileno);

   ACE_OS::fputs("\n######## Backtrace Done #######\n",fp);

   ACE_OS::fclose(fp);
}//end printStackTraceToFile


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

