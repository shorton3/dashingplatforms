/******************************************************************************
* 
* File name:   ResourceMonitor.h 
* Subsystem:   Platform Services 
* Description: Resource Monitor provides auditing of critical card resources,
*              for example disk usage, memory usage, and cpu usage.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_RESOURCE_MONITOR_H_
#define _PLAT_RESOURCE_MONITOR_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/OS.h>
#include <ace/Reactor.h>
#include <ace/Signal.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OSResource.h"

#include "platform/msgmgr/MailboxAddress.h"
#include "platform/msgmgr/MailboxOwnerHandle.h"
#include "platform/msgmgr/MessageHandlerList.h"

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
 * The ResourceMonitor provides auditing of critical card resources.
 * <p>
 * ResourceMonitor monitors cpu, memory, and disk usage as well
 * as other critical resources. Historical trend data is stored for
 * these resources, and overload control threshold are monitored to 
 * alarm unhealthy conditions.
 * <p>
 * ResourceMonitor accepts the following command line options:
 * -l Local Logger Mode only (do not enqueue logs to shared memory)
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ResourceMonitorTimerMessage;

class ResourceMonitor
{
   public:

      /** Constructor */
      ResourceMonitor();

      /** Virtual Destructor */
      virtual ~ResourceMonitor();

      /** Start the mailbox processing loop */
      void processMailbox();

      /** Catch the shutdown signal and begin the graceful shutdown */
      static void catchShutdownSignal();

      /** Perform ResourceMonitor initialization */
      int initialize(int argc, ACE_TCHAR *argv[]);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /** Retrieve the static singleton instance of the ResourceMonitor */
      static ResourceMonitor* getInstance();

      /** Start the reactor processing thread for signal handling, etc. */
      static void startReactor();

      /** Gracefully shutdown the ResourceMonitor and all of its threads */
      void shutdown();

      /**
       * Called in the constructor to setup the mailbox, activate and
       * to setup handlers.
       * @param remoteAddress Mailbox Address of the ResourceMonitor DistributedMailbox
       * @returns true if successful
       */
      bool setupMailbox(const MailboxAddress& remoteAddress);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ResourceMonitor(const ResourceMonitor& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ResourceMonitor& operator= (const ResourceMonitor& rhs);

      /** Static singleton instance */
      static ResourceMonitor* resourceMonitor_;

      /**
       * Message Handler for Timer Messages. Here these are transferred via the
       * local mailbox only.
       */
      int processTimerMessage(MessageBase* message);

      /** ResourceMonitor Mailbox */
      MailboxOwnerHandle* resourceMonitorMailbox_;

      /** Message Handler List for storing Functors for Mailbox handlers */
      MessageHandlerList* messageHandlerList_;

      /** OS Resource Monitoring module */
      OSResource osResourceMonitor_;

      /** Monitoring Timer Id */
      long monitoringTimerId_;

      /** Monitoring Timer Message */
      ResourceMonitorTimerMessage* monitoringTimerMessage_;

      /** Signal Adapter for registering signal handlers */
      ACE_Sig_Adapter* signalAdapter_;

      /** Signal Set for registering signal handlers */
      ACE_Sig_Set signalSet_;

      /** ACE_Select_Reactor used for signal handling */
      static ACE_Reactor* selectReactor_;
};

#endif
