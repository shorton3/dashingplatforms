/******************************************************************************
*
* File name:   ProcessController.cpp
* Subsystem:   Platform Services
* Description: The purpose of this class is to create an ACE_Service_Object
*              subclass to allow dynamic loading/unloading of our application
*              processes. This class acts as a bridge pattern between the
*              Service Configurator(Service Object) and the ACE Process_Manager
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

#include <cstring>
#include <cstdio>
#include <iostream>
#include <sstream>

#include <ace/Reactor.h>
#include <ace/Process_Manager.h>
#include <ace/Time_Value.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ProcessController.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#define PROCESS_EXIT_DELAY 15

// Statically create the ACE Process Manager
ACE_Process_Manager* ProcessController::processManager_ = ACE_Process_Manager::instance();

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ProcessController::ProcessController(): childProcessPID_(ACE_INVALID_PID),
                                        processName_(""),
                                        processDeathHandler_(NULL)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ProcessController::~ProcessController()
{
   delete processDeathHandler_;
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform service startup; here we will start the process which 
//              is passed to this instance in these parameter arguments (which
//              originate from svc.conf). Inherited from ACE_Service_Object
// Design:
//-----------------------------------------------------------------------------
int ProcessController::init(int argc, ACE_TCHAR* argv[])
{
   // extract the first argument as this is the Process filename;
   // NOTE that the 2nd argument must the file name to redirect stderr/stdout to
   // or it must be 'NULL'
   if (argc > 1)
   {
      ostringstream ostr;
      // Build the command line
      for (int i = 0; i < argc; i++)
      {
         // Skip over the 2nd argument since that tells whether or not IO is
         // redirected to a file or set to NULL (standard io mechanism). Note this
         // is set in this file, no need to pass to the child process
         if (i != 1)
         {
            ostr << argv[i];
            if ((i < (argc - 1)) && (argc != 2))
            {
               // Add a space between process executable name and arguments, but
               // not after the last argument
               ostr << " ";
            }//end if
         }//end if
      }//end for
      //ostr << ends;

      // By default the child process inherits the environment of the parent
      processOptions_.command_line(ostr.str().c_str());

      // Extract the process executable name
      processName_ = processOptions_.process_name(); 
  
      // Redirect the child process stdin/stdout/stderr to our file (as long as NULL was
      // not specified in the svc.conf file)
      if (strcmp(argv[1],"NULL") != 0)
      {
         // Note ACE_DEFAULT_FILE_PERMS is 0644
         ACE_HANDLE outputfd = ACE_OS::open (argv[1], O_RDWR | O_CREAT | O_APPEND, ACE_DEFAULT_FILE_PERMS);
         if (processOptions_.set_handles(ACE_INVALID_HANDLE, outputfd, outputfd) == ERROR)
         {
            TRACELOG(ERRORLOG, PROCMGRLOG, "Unable to redirect child process output handles",0,0,0,0,0,0);
         }//end if
      }//end if

      // Spawn the process - Store the child process PID so we can use it to
      // respond to changes in the service configuration file 
      childProcessPID_ = processManager_->spawn(processOptions_);
      if (childProcessPID_ == ACE_INVALID_PID)
      {
         ostringstream ostr2;
         ostr2 << "Unable to spawn child process " << processName_ << ends;
         STRACELOG(ERRORLOG, PROCMGRLOG, ostr2.str().c_str());
         return ERROR;
      }//end if
      else
      {
         ostringstream ostr2;
         ostr2 << "Successfully spawned process " << processName_ << " with PID=" << childProcessPID_
               << ", and process options <" << ostr.str().c_str() << ">, now registering death handler" << ends;
         STRACELOG(DEBUGLOG, PROCMGRLOG, ostr2.str().c_str());
      }//end else

      // Create the Death Handler for the child process and pass-in the restart method (only do this
      // for the first initialization, then we re-use the death handler object
      if (processDeathHandler_ == NULL)
      {
         RestartProcessMethod restartMethod = makeFunctor((RestartProcessMethod*)0, *this, &ProcessController::restartProcess);
         processDeathHandler_ = new ProcessDeathHandler(restartMethod);
      }//end if
      else
      {
         processDeathHandler_->setTerminatedFlag(false);
      }//end else

      // Configure the death handler for the child pid and process name
      processDeathHandler_->setProcessIdentity(childProcessPID_, processName_);

      // Register the death handler for notifications
      if (processManager_->register_handler (processDeathHandler_, childProcessPID_) == ERROR)
      {
         TRACELOG(ERRORLOG, PROCMGRLOG, "Error registering process death handler",0,0,0,0,0,0);
      }//end if
   }//end if
   else
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Empty arguments passed",0,0,0,0,0,0);
      return ERROR;
   }//end else 
   return OK;
}//end init


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Inherited from ACE_Service_Object. Perform service shutdown; 
//              here we destroy the process associated with this instance
// Design:
//-----------------------------------------------------------------------------
int ProcessController::fini()
{
   ostringstream ostr;
   ostr << "Terminating (fini) process " << processName_ << ". " << processManager_->managed()
        << " managed processes remaining." << ends;
   STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());

   // Tell the Process Death Handler that this process has been purposefully terminated
   // (so it doesn't try to restart the process)
   processDeathHandler_->setTerminatedFlag(true);

   // Kill the process but give it time to clean up (use SIGINT). Here, an
   // empty 2nd parameter means abort the process and terminate immediately.
   // This also means that we have to install a SIGINT signal handler in
   // each application
   processManager_->terminate(childProcessPID_, SIGINT); 

   // Here we wait to reap the exit status of the child process
   ACE_Time_Value tv(PROCESS_EXIT_DELAY,0);
   pid_t result = processManager_->wait(childProcessPID_, tv);
   if (result == childProcessPID_)
   {
      ostringstream ostr;
      ostr << "Process " << processName_ << " exitted successfully. " << processManager_->managed()
           << " managed processes remaining." << ends;
      STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());
   }//end if
   else if (result == 0)
   {
      ostringstream ostr;
      ostr << "Timeout occurred while waiting for process " << processName_ << " to exit. " << processManager_->managed()
           << " managed processes remaining." << ends;
      STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());
   }//end else if
   return OK;
}//end fini


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Inherited from ACE_Service_Object. Return service descriptive
//              information and state data
// Design:
//-----------------------------------------------------------------------------
int ProcessController::info(ACE_TCHAR** infoString, size_t size) const
{
   strncpy(*infoString, processName_.c_str(), size);
   return OK;
}//end info


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Inherited from ACE_Service_Object. Suspend activity for this 
//              Service/Process
// Design:
//-----------------------------------------------------------------------------
int ProcessController::suspend()
{
   // NOT YET IMPLEMENTED: This method to be implemented via MsgMgr mailbox IPC
   // -or- via Unix Signals

   ostringstream ostr;
   ostr << "Requested suspend process " << processName_ << ends;
   STRACELOG(DEBUGLOG, PROCMGRLOG, ostr.str().c_str());

   return OK;
}//end suspend


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Inherited from ACE_Service_Object. Resume activity for this 
//              Service/Process
// Design:
//-----------------------------------------------------------------------------
int ProcessController::resume()
{
   // NOT YET IMPLEMENTED: This method to be implemented via MsgMgr mailbox IPC
   // -or- via Unix Signals

   ostringstream ostr;
   ostr << "Requested resume process " << processName_ << ends;
   STRACELOG(DEBUGLOG, PROCMGRLOG, ostr.str().c_str());

   return OK;
}//end resume


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Attempt a process restart. This method is called from the 
//              Process Death Handler.
// Design:
//-----------------------------------------------------------------------------
int ProcessController::restartProcess()
{
   ostringstream ostr;
   ostr << "Requested restart process <" << processName_ << "> with Process Options: <" 
        << processOptions_.command_line_buf() << ">" << ends;
   STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());

   // Spawn the process - Store the child process PID so we can use it to
   // respond to changes in the service configuration file
   childProcessPID_ = processManager_->spawn(processOptions_);
   if ( (childProcessPID_ == ACE_INVALID_PID) || (childProcessPID_ == 0) )
   {
      ostringstream ostr;
      ostr << "Unable to spawn child process " << processName_ << ends;
      STRACELOG(ERRORLOG, PROCMGRLOG, ostr.str().c_str());
      return ERROR;
   }//end if

   // Create the Death Handler for the child process
   processDeathHandler_->setProcessIdentity(childProcessPID_, processName_);

   // Register the death handler for notifications
   if (processManager_->register_handler (processDeathHandler_, childProcessPID_) == ERROR)
   {
      TRACELOG(ERRORLOG, PROCMGRLOG, "Error registering process death handler",0,0,0,0,0,0);
   }//end if
   return OK;
}//end restartProcess


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string ProcessController::toString()
{
   string s = "";
   return (s);
}//end toString


ACE_FACTORY_DEFINE(ProcessController, ProcessController);

//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

