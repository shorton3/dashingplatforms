/******************************************************************************
*
* File name:   CliMenuStateActions.h
* Subsystem:   EMS
* Description: Static handler methods for the State Transition Command
*              Menu Items.
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

#include <tao/ORB_Core.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliMenuStateActions.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
CliMenuStateActions::CliMenuStateActions()
{
}//end Constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
CliMenuStateActions::~CliMenuStateActions()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize static variables to be used by the methods
// Design:     
//-----------------------------------------------------------------------------
void CliMenuStateActions::initialize(session::Session_I_ptr session_I_ptr)
{
   session_I_ptr_ = session_I_ptr;
   //session_I_ptr_ = session::Session_I::_duplicate(session_I_ptr);
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Send an Unlock State Command
// Design:
//-----------------------------------------------------------------------------
void CliMenuStateActions::sendUnlock()
{
   char* exceptionMessage = NULL;

   if (CORBA::is_nil (session_I_ptr_) || session_I_ptr_->_non_existent())
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "SessionIPtr is NULL",0,0,0,0,0,0);
      return;
   }//end if

   try
   {
      exceptionMessage = "While sending Unlock State Transition";
// REPLACE THIS WITH A REAL CALL FOR A STATE TRANSITION
      session_I_ptr_->ping();
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
   }//end catch
}//end sendUnlock


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Send a Lock State Command
// Design:
//-----------------------------------------------------------------------------
void CliMenuStateActions::sendLock()
{
   char* exceptionMessage = NULL;

   if (CORBA::is_nil (session_I_ptr_) || session_I_ptr_->_non_existent())
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "SessionIPtr is NULL",0,0,0,0,0,0);
      return;
   }//end if

   try
   {
      exceptionMessage = "While sending Unlock State Transition";
// REPLACE THIS WITH A REAL CALL FOR A STATE TRANSITION
      session_I_ptr_->ping();
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
   }//end catch/end sendLock
}//end sendLock


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

