/******************************************************************************
* 
* File name:   ProcessController.h 
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

#ifndef _PLAT_PROCESS_CONTROLLER_H_
#define _PLAT_PROCESS_CONTROLLER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Global_Macros.h>
#include <ace/Service_Object.h>
#include <ace/Process.h>
#include <ace/OS_NS_unistd.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ProcessDeathHandler.h"
#include "ProcessController_Export.h"

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
 * The purpose of this class is to create an ACE_Service_Object
 * subclass to allow dynamic loading/unloading of our application
 * processes. This class acts as a bridge pattern between the 
 * Service Configurator(Service Object) and the ACE Process_Manager
 * <p>
 * Each instance of this class corresponds to a configured Service/Process
 * in the system.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ProcessController_Export ProcessController : public ACE_Service_Object
{

   public:

      /** Constructor */
      ProcessController();

      /** Virtual Destructor */
      virtual ~ProcessController();
 
      /** Inherited from ACE_Service_Object. Perform service startup; here we
          will start the process which is passed to this instance in these
          parameter arguments (which originate from svc.conf) */
      int init(int argc, ACE_TCHAR* argv[]);

      /** Inherited from ACE_Service_Object. Perform service shutdown; here we
          destroy the process associated with this instance */
      int fini();

      /** Inherited from ACE_Service_Object. Return service descriptive
          information and state data */
      int info(ACE_TCHAR**, size_t) const;

      /** Inherited from ACE_Service_Object. Suspend activity for this Service/Process */
      int suspend();

      /** Inherited from ACE_Service_Object. Resume activity for this Service/Process */
      int resume();

      /** Attempt a process restart. This method is called from the Process Death Handler. */
      int restartProcess();

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
      ProcessController(const ProcessController& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ProcessController& operator= (const ProcessController& rhs);

      /** Static instance of the ACE Process Manager */
      static ACE_Process_Manager* processManager_;

      /** ACE Process Options. This structure is populated from parameters received
          when the Service Configurator calls the init() method. This data is stored
          here for process restarts */
      ACE_Process_Options processOptions_;

      /** OS PID of the spawned child process */
      pid_t childProcessPID_;

      /** Name of the child process executable */
      string processName_;

      /** Pointer to the Process Death Handler that gets created for each
          instance of the ProcessController (for handling process death and restart) */
      ProcessDeathHandler* processDeathHandler_;
};

ACE_FACTORY_DECLARE (ProcessController, ProcessController)

#endif
