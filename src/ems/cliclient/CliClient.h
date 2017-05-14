/******************************************************************************
* 
* File name:   CliClient.h 
* Subsystem:   EMS
* Description: Provides a CLI Client interface to the EMS' Client Agent 
*              through CORBA.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _EMS_CLI_CLIENT_H_
#define _EMS_CLI_CLIENT_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliMenuController.h"
#include "ems/idls/cppsource/sessionC.h"
#include "sessionI.h"
#include "ems/idls/cppsource/platformConfigC.h"
#include "ems/idls/cppsource/alarmsC.h"

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
 * Provides a CLI Client interface to the EMS' Client Agent through CORBA.
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
 * CliClient accepts the following startup options:
 * -h Restricts the use of hostnames in IORs. Invokes -ORBDottedDecimalAddresses 1 on the Orb
 * -d Enable debug messages from within the Orb. Invokes -ORBDebug on the Orb.
 * -l <Debug Level> Sets the debug level within the Orb (Max is 10). Invokes -ORBDebugLevel on the Orb.
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
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class CliClient
{
   public:

      /** Constructor */
      CliClient();

      /** Virtual Destructor */
      virtual ~CliClient();

      /** Gracefully shutdown the CliClient and all of its threads */
      static void shutdown();

      /** 
       * Perform CliClient initialization
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

      /** Handle Menu Option user input */
      void processMenu();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /** Method for starting the Tao Orb run loop in a separate thread */
      static void runOrb();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      CliClient(const CliClient& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      CliClient& operator= (const CliClient& rhs);

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
      int resolveAgentObjects();

      /** The ORB Manager */
      TAO_ORB_Manager orbManager_;

      /** Naming Service context */
      CosNaming::NamingContext_var namingContextVar_;

      /** Session IDL Interface Object - Session instance that Client binds to Naming Service */
      session_Session_I_i* sessionImplPtr_;

      /** Agent Session IDL interface object - HERE we store for ONLY ONE AGENT. Needs to be
          changed to support multiple redundant agents */
      session::Session_I_ptr session_I_ptr_;

      /** Platform Config remote reference object */
      platformConfig::platformConfig_I_ptr platformConfig_I_ptr_;

      /** Agent Alarms remote reference object */
      alarms::alarmInterface_ptr alarmInterface_ptr_;

      /** Menu manager */
      CliMenuController cliMenuController_;
};

#endif
