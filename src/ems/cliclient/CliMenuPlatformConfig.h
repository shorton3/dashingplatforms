/******************************************************************************
* 
* File name:   CliMenuPlatformConfig.h 
* Subsystem:   EMS
* Description: Static handler methods for platform configuration requests.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _EMS_CLI_MENU_PLATFORM_CONFIG_H_
#define _EMS_CLI_MENU_PLATFORM_CONFIG_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliMenuBaseActions.h"
#include "ems/idls/cppsource/platformConfigC.h"

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
 * CliMenuPlatformConfig contains static handler methods for platform configuration requests.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class CliMenuPlatformConfig : public CliMenuBaseActions
{
   public:

      /** Constructor */
      CliMenuPlatformConfig();

      /** Virtual Destructor */
      virtual ~CliMenuPlatformConfig();

      /** Initialization Method */ 
      void initialize(platformConfig::platformConfig_I_ptr platformConfig_I_ptr);

      /** Retrieve all of the subsystem / severity log level pairs from the agent */
      string getLogLevels();

      /** Set the log level severity based on the subsystem id */
      void setLogLevel(int subsystem, int severityLevel);

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      CliMenuPlatformConfig(const CliMenuPlatformConfig& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      CliMenuPlatformConfig& operator= (const CliMenuPlatformConfig& rhs);

      /** Pointer to the Platform Config interface object. Here we use a pointer since
          we're passing in the reference from the main class that handles the Corba stuff */
      platformConfig::platformConfig_I_ptr platformConfig_I_ptr_;

      /** Default error string */
      string errorString_;

};

#endif
