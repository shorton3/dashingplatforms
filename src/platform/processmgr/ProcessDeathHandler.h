/******************************************************************************
* 
* File name:   ProcessDeathHandler.h 
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

#ifndef _PLAT_PROCESS_DEATH_HANDLER_H_
#define _PLAT_PROCESS_DEATH_HANDLER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Event_Handler.h>
#include <ace/OS_NS_unistd.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/msgmgr/Callback.h"

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

// For C++ class declarations, we have one (and only one) of these access 
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors/destructors first)
// 5) instance data
//

/**
 * ProcessDeathHandler implements an ACE Event Handler to handle the ACE
 * Process Manager notifications for processes that have died. 
 * <p>
 * It was necessary to handle this in a separate class since the
 * ProcessController class inherits from Service_Object, which is
 * itself an EventHandler. An instance of the ProcessDeathHandler
 * exists for each process (ProcessController) spawned by the 
 * Service Configurator.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

typedef CBFunctor0wRet<int> RestartProcessMethod;

class ProcessDeathHandler: public ACE_Event_Handler
{

   public:

      /** Constructor */
      ProcessDeathHandler(RestartProcessMethod& restartProcessMethod);

      /** Virtual Destructor */
      virtual ~ProcessDeathHandler();

      /** ACE Event Handler implementation */
      virtual int handle_exit (ACE_Process* process);

      /** ACE Event Handler implementation */
      virtual int handle_input (ACE_HANDLE pid);

      /** ACE Event Handler implementation */
      virtual int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask);

      /** ACE Event Handler implementation */
      virtual int handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0);

      /** Initialize the process identity info (ID and Name) */
      void setProcessIdentity(pid_t processID, string& processName);

      /** Method to set/reset whether or not this process has been manually terminated */
      void setTerminatedFlag(bool hasBeenTerminated);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ProcessDeathHandler(const ProcessDeathHandler& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ProcessDeathHandler& operator= (const ProcessDeathHandler& rhs);

      /** Counter for determining the number of deaths per process that have occurred */
      unsigned short counter_;

      /** PID associated with this instance of the Death Handler */
      pid_t processID_;

      /** Process Name associated with this instance of the Death Handler */
      string processName_;

      /** Tightly coupled pointer to the associated Process Controller */
      const RestartProcessMethod restartProcessMethod_;

      /** Flag to indicated that this process has been manually terminated */
      bool hasBeenTerminated_;
};

#endif
