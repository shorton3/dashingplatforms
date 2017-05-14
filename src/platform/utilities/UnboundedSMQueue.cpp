/******************************************************************************
*
* File name:   UnboundedSMQueue.cpp
* Subsystem:   Platform Services
* Description: Explicit template instantiation
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

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "UnboundedSMQueue.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Utilities Library"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class UnboundedSMQueue<int>;
template class ACE_Unbounded_Queue<int>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate UnboundedSMQueue<int>
#pragma instantiate ACE_Unbounded_Queue<int>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

