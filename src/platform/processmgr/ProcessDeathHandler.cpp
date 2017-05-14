/******************************************************************************
*
* File name:   ProcessDeathHandler.cpp
* Subsystem:   Platform Services
* Description: This class implements an ACE Event Handler to handle the ACE
*              Process Manager notifications for processes that have died. It
*              was necessary to handle this in a separate class since the
*              ProcessController class inherits from Service_Object, which is
*              itself an EventHandler.
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

#include <sstream>

#include <ace/Process.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ProcessDeathHandler.h"

#include "platform/logger/Logger.h"

#include "platform/common/Defines.h"

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
ProcessDeathHandler::ProcessDeathHandler(RestartProcessMethod& restartProcessMethod) 
                    : counter_(0),
                      processID_(0),
                      processName_(""),
                      restartProcessMethod_(restartProcessMethod),
                      hasBeenTerminated_(false)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ProcessDeathHandler::~ProcessDeathHandler()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Event Handler implementation handles death notifications 
// Design:
//-----------------------------------------------------------------------------
int ProcessDeathHandler::handle_exit (ACE_Process* process)
{
   ostringstream ostr;
   // Check to see if the process is still running
   if (process->running() == 1)
   {
      ostr << "Process death detected for process " << processName_
           << "(" << process->getpid() << "), but process is still running" << ends;
   }//end if
   else
   {
      ostr << "Process death detected for process " << processName_ 
           << "(" << process->getpid() << ") with return code (" << process->return_value()
           << ") and exit code (" << process->exit_code() << ")" << ends;
   }//end else
   STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());

   // If this process has been manually terminated, then do NOT attempt a restart
   if (hasBeenTerminated_ == true)
   {
      return OK;
   }//end if

   // If the process death has re-occurred too many times, reboot
   if (++counter_ >= 3)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Rebooting system due to too many process failures",0,0,0,0,0,0);
      //system("/sbin/reboot");
   }//end if
   else
   {
      ostringstream ostr;
      ostr << "Attempting to restart process " << processName_ << " " << counter_ << " number of times" << ends;
      STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());
      // Attempt to restart the process
      if (restartProcessMethod_() == ERROR)
      {
         ostringstream ostr;
         ostr << "Error restarting process " << processName_ << ends;
         STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());
      }//end if
   }//end else
   return OK;
}//end handle_exit


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Event Handler implementation handles death notifications
// Design:
//-----------------------------------------------------------------------------
int ProcessDeathHandler::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
   ostringstream ostr;
   ostr << "Process close handled for process " << processName_ << ends;
   STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());

   // Handle the unused variable warning
   handle = 0;
   close_mask = 0;
   return OK;
}//end handle_close


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Event Handler implementation handles death notifications
// Design:
//-----------------------------------------------------------------------------
int ProcessDeathHandler::handle_signal (int signum, siginfo_t *, ucontext_t *)
{
   ostringstream ostr;
   ostr << "Process signal handled for process " << processName_ << ends;
   STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());
 
   // Handle the unused variable warning
   signum = 0;
   return OK;
}//end handle_signal


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Event Handler implementation handles death notifications
// Design:
//-----------------------------------------------------------------------------
int ProcessDeathHandler::handle_input (ACE_HANDLE pid)
{
   ostringstream ostr;
   ostr << "Process input handled for process " << processName_ << "(" << pid << ")" << ends;
   STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());

   return OK;
}//end handle_input


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize the process identity info (ID and Name)
// Design:
//-----------------------------------------------------------------------------
void ProcessDeathHandler::setProcessIdentity(pid_t processID, string& processName)
{
   processID_ = processID;
   processName_ = processName;
}//end setProcessIdentity


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to set/reset whether or not this process has been manually terminated 
// Design:
//-----------------------------------------------------------------------------
void ProcessDeathHandler::setTerminatedFlag(bool hasBeenTerminated)
{
   hasBeenTerminated_ = hasBeenTerminated;
}//end setTerminatedFlag


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string ProcessDeathHandler::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

