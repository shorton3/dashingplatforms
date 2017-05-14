/******************************************************************************
* 
* File name:   ProcessManager.h 
* Subsystem:   Platform Services 
* Description: The process manager provides for the configuration and startup 
*              of all platform and application processes and process monitoring
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_PROCESS_MANAGER_FILE_H_
#define _PLAT_PROCESS_MANAGER_FILE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Process_Manager.h>
#include <ace/Reactor.h>
#include <ace/Signal.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/msgmgr/MessageHandlerList.h"
#include "platform/msgmgr/MailboxOwnerHandle.h"

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
 * The process manager provides for the configuration and startup
 * of all platform and application processes as well as process monitoring 
 * <p>
 * ProcessManager makes use of the ACE Service Configurator class
 * to manage passing configuration information to the processes. The Service
 * Configurator manages a configuration file (with default name 'svc.conf'),
 * and it provides for the dynamic reconfiguration of processes using the SIGHUP 
 * signal. The Service Configurator provides interfaces for starting,
 * stopping, pausing, resuming, reconfiguring etc. the services (processes)
 * on-the-fly without stopping the whole system. This feature forms the basis
 * for the system patch management strategy.
 * <p>
 * ProcessManager also makes use of the ACE_Process_Manager class to handle
 * spawning of processes, monitoring child process death, etc.
 * <p>
 * ProcessManager accepts the following startup options:
 * -b Daemonize the Process Manager process
 * -d Verbose diagnostics as process directives are forked
 * -f Read a different file other than svc.conf
 * -n Do not process static directives
 * -s Designate which signal to use for reconfigure instead of SIGHUP
 * -S Pass in a process directive not specified in the svc.conf file
 * -y Process static directives
 * -l Local Logger Mode only (do not enqueue logs to shared memory)
 * -r Redirect stdout/stderr to a specified file
 * <p>
 * The svc.conf configuration file that drives the Process Manager accepts
 * the following format. For statically linked services (these are optional
 * features that will run within the Process Manager OS process):
 *   static <service name> ["argc/argv options for the service"]
 * For dynamically linked services, for example, those services intended 
 * to run within their own OS process:
 *   dynamic <service name> <service type> <DLL/.so name>:<factory_function> ["argc/argv options"]
 * where the 'service type' is either a Service_Object *, Module *, or Stream *
 * -and-
 * where the "argc/argv options" have the following format:
 *   "<process executable name> <'NULL' or redirected IO filename> <process options>"
 * The following additional service directives are supported:
 *   remove <service name>          shuts down the service/process
 *   suspend <service name>         suspends activity for the process
 *   resume <service name>          resumes activity for the process
 * Note that "argc/argv options" 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ProcessManager
{
   public:

      /** Constructor */
      ProcessManager();

      /** Virtual Destructor */
      virtual ~ProcessManager();

      /** Start the mailbox processing loop */
      void processMailbox();

      /** Method to catch OS signals */
      static void catchShutdownSignal();

      /** 
       * This method starts the reactor thread needed for processing
       * Process Death Handlers and auto-reaping the exit statuses of the
       * child processes.
       */
      static void startReactor();

      /** Perform Process Manager initialization */
      int initialize(int argc, ACE_TCHAR *argv[], string usageString);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /** Retrieve static singleton instance */
      static ProcessManager* getInstance();

      /** Gracefully shutdown the Process Manager and all of its child processes */
      void shutdown();

      /**
       * Called in the constructor to setup the mailbox, activate and
       * to setup handlers.
       * @param remoteAddress Mailbox Address of the ProcessManager LocalMailbox
       * @returns true if successful
       */
      bool setupMailbox(const MailboxAddress& localAddress);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ProcessManager(const ProcessManager& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ProcessManager& operator= (const ProcessManager& rhs);

      /** ProcessManager Mailbox */
      MailboxOwnerHandle* processManagerMailbox_;
                                                                                                       
      /** Message Handler List for storing Functors for Mailbox handlers */
      MessageHandlerList* messageHandlerList_;

      /** Pointer to the underlying ACE Process Manager */
      ACE_Process_Manager* processManager_;

      /** Signal Adapter for registering signal handlers */
      ACE_Sig_Adapter* signalAdapter_;

      /** Signal Set for registering signal handlers */
      ACE_Sig_Set signalSet_;

      /** ACE_Select_Reactor used for signal handling and Process Manager */
      static ACE_Reactor* selectReactor_;

      /** Static singleton instance */
      static ProcessManager* ourProcessManager_;

};

#endif
