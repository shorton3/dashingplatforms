/******************************************************************************
* 
* File name:   ClientAgent.h 
* Subsystem:   EMS
* Description: CORBA Enabled Agent for receiving and processing client commands
*              from the various client interfaces.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _EMS_CLIENT_AGENT_H_
#define _EMS_CLIENT_AGENT_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <fstream>
#include <string>

#include <ace/Reactor.h>
#include <ace/Signal.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ems/idls/cppsource/sessionC.h"
#include "sessionI.h"
#include "ems/idls/cppsource/platformConfigC.h"
#include "platformConfigI.h"
#include "ems/idls/cppsource/alarmsC.h"
#include "alarmsI.h"

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
 * CORBA Enabled Agent for receiving and processing client commands
 *  from the various client interfaces.
 * <p>
 * ClientAgent implements the CORBA Server for handling the various
 * client interfaces. This module uses the ACE/TAO Corba Orb, which
 * is fully OMG CORBA 2.3 compliant.
 * <p>
 * Note: Previous versions of the TAO Orb used TAO Exception Handling
 * Macros to make the Standard Corba Exceptions compatible with platforms
 * that do not support native C++ exception handling constructs. These
 * TAO specific Macros have now been deprecated (they now cause compiler
 * errors); they have been replaced with the base ACE Exception Handling
 * Macros. However, since our development platform WILL ALWAYS support
 * native C++ exception handling, we are foregoing the use of the ACE
 * Macros, and we will ONLY support the native exception handling for 
 * simplicity.
 * <p>
 * ClientAgent accepts the following startup options:
 * -l Local Logger Mode only (do not enqueue logs to shared memory)
 * -h Restricts the use of hostnames in IORs. Invokes -ORBDottedDecimalAddresses 1 on the Orb
 * -d Enable debug messages from within the Orb. Invokes -ORBDebug on the Orb.
 * -v <Debug Level> Sets the debug level within the Orb (Max is 10). Invokes -ORBDebugLevel on the Orb.
 * -f <log filename> Sets the log filename for Orb debug logs. Invokes -ORBLogFile on the Orb.
 *      (Default is stdout)
 * -e <Endpoint> Orb listens for connections on the interfaces specified by the endpoint URL 
 *      (Invokes -ORBListenEndpoints)
 * -i <Initial Reference> URL the Orb uses for initial resolution of Naming Service (corbaloc)
 *      (Invokes -ORBInitRef)
 * -r <Default Init Ref> URL the Orb uses for default initial resolution of Naming Service (corbaloc)
 *      (Invokes -ORBDefaultInitRef)
 * <p>
 * Note that the correct syntax for -i (the ORBInitRef) -ORBInitRef is:
 *   NameService=corbaloc:iiop:localhost:12345/NameService
 *     -- or --
 *   NameService=iiop://localhost:12345/NameService
 * The former (corbaloc) syntax is the standard (compliant with the Interoperable Naming
 * Service specification) syntax. The latter syntax is TAO-specific and is deprecated. 
 * <p>
 * Client Agent implements the following IDL interfaces:
 * -session Performs heartbeat monitoring (bidirectionally) between agent and its clients
 * -platformConfig Handles get/set requests for the following platform configuration data sections:
 *    -- Log Level configuration, When updating log levels at runtime, the Agent will set the 
 *       the new log levels in shared memory for all of the Log clients to use as well as set
 *       the same values in the database for persistence. These log level values will be
 *       read by the Agent upon startup each time and set in shared memory.
 * -alarms Alarm Management and notification interface
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ClientAgent
{
   public:

      /** Constructor */
      ClientAgent();

      /** Virtual Destructor */
      virtual ~ClientAgent();

      /** Start the mailbox processing loop */
      void processMailbox();

      /** Catch the shutdown signal and begin the graceful shutdown */
      static void catchShutdownSignal();

      /** 
       * Perform ClientAgent initialization 
       * @param dottedDecimalIORFlag If true, fore-go the use of hostnames in IORs.
       *    Instead use IP Addresses
       * @param debugFlag If true, enable the Orb inner debug logging
       * @param debugLevel Max is 10 -- the most verbose logging
       * @param debugFilename Redirect Orb debug logs to a file instead of stdout
       * @param endPointString On a multihomed host, instruct the Orb to use the
       *    interface described by this endpoint
       * @param initRef Use the Interoperable Naming Service url for locating
       *    Naming Service reference
       * @param defaultInitRef Use the INS url for default reference resolving
       * @returns ERROR upon failure; otherwise OK
       */
      int initialize(bool dottedDecimalIORFlag, bool debugFlag, string& debugLevel, string& debugFilename,
           string& endPointString, string& initRef, string& defaultInitRef);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /** Retrieve the static singleton instance of the ClientAgent */
      static ClientAgent* getInstance();

      /** Method for starting the Tao Orb run loop in a separate thread */
      static void runOrb();

      /** Start the reactor processing thread for signal handling, etc. */
      static void startReactor();

      /** Gracefully shutdown the ClientAgent and all of its threads */
      void shutdown();

      /**
       * Called in the constructor to setup the mailbox, activate and
       * to setup handlers.
       * @param remoteAddress Mailbox Address of the ClientAgent DistributedMailbox
       * @returns true if successful
       */
      bool setupMailbox(const MailboxAddress& remoteAddress);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ClientAgent(const ClientAgent& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ClientAgent& operator= (const ClientAgent& rhs);

      /**
       * Perform instantiation and POA registration of IDL-specific interface objects.
       * Here, we put the changes and new additions for IDLs and IDL features
       * @param rootPOA reference to the root POA for object registration and activation.
       * @returns OK on success; otherwise ERROR
       */
      int createIDLObjects(PortableServer::POA_var& rootPOA);

      /**
       * Perform initial resolution of the Naming Service. Then, bind our top level
       * objects into the Naming Service. 
       * @returns OK on success; otherwise ERROR
       */
      int initNamingService();

      /**
       * Resolve/retrieve our clients' top level interface objects from the Naming Service
       * @returns OK on success; otherwise ERROR
       */
      int resolveClientObjects();

      /**
       * Handle receiving AlarmEvent Messages.
       * Alarms/Clear Alarms are updated into the system-wide outstanding alarms table,
       * and then they are forwarded to the clients/NOC. Event Reports are written to
       * a rolling file.
       */
      int processAlarmEventMessage(MessageBase* message);

      /** Static singleton instance */
      static ClientAgent* clientAgent_;

      /** ClientAgent Mailbox */
      MailboxOwnerHandle* clientAgentMailbox_;

      /** Message Handler List for storing Functors for Mailbox handlers */
      MessageHandlerList* messageHandlerList_;

      /** The ORB Manager */
      TAO_ORB_Manager orbManager_;

      /** Naming Service context */
      CosNaming::NamingContext_var namingContextVar_;

      /** Session IDL Interface Object - Session instance that Agent binds to Naming Service */
      session_Session_I_i* sessionImplPtr_;
      
      /** Client Session IDL interface object - HERE we store for ONLY ONE CLIENT. Needs to be
          changed to support multiple clients */
      session::Session_I_ptr session_I_ptr_;

      /** Platform Config Server Interface Object - PlatformConfig instance that Agent bind to 
          the Naming Service */
      platformConfig_platformConfig_I_i* platformConfigImplPtr_;

      /** Alarms IDL Interface Object - Alarms instance that Agent binds to Naming Service */
      alarms_alarmInterface_i* alarmImplPtr_;

//TODO:
      /** Client Alarms IDL interface object - HERE we store for ONLY ONE CLIENT. Needs to be
          changed to support multiple clients. This is the interface we use to send alarm 
          notifications to the clients */
      alarms::alarmInterface_ptr alarmInterface_ptr_;

      /** Signal Adapter for registering signal handlers */
      ACE_Sig_Adapter* signalAdapter_;

      /** Signal Set for registering signal handlers */
      ACE_Sig_Set signalSet_;

      /** NEID (Network Element Id) of this local node (9 characters) */
      string localNEID_;

      /** Pointer to an output file stream for event reports  */
      fstream eventReportFileStream_;

      /** Event Report file name string */
      string eventReportFileName_;

      /** Counter for determining how many event reports get processed between checking 
          the size of the output file */
      int fileCheckCounter_;

      /** Number of event report files to keep and maintain as part of the rollover */
      int numberKeptEventFiles_;

      /** Maximum approximate size of each event report file allowed before rollover */
      int sizeKeptEventFiles_;

      /** ACE_Select_Reactor used for signal handling */
      static ACE_Reactor* selectReactor_;
};

#endif
