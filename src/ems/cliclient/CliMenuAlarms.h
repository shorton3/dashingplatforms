/******************************************************************************
* 
* File name:   CliMenuAlarms.h 
* Subsystem:   EMS
* Description: Static handler methods for alarm requests.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _EMS_CLI_MENU_ALARMS_H_
#define _EMS_CLI_MENU_ALARMS_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliMenuBaseActions.h"
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
 * CliMenuAlarms contains static handler methods for alarm requests.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class CliMenuAlarms : public CliMenuBaseActions
{
   public:

      /** Constructor */
      CliMenuAlarms();

      /** Virtual Destructor */
      virtual ~CliMenuAlarms();

      /** Initialization Method */ 
      void initialize(alarms::alarmInterface_ptr alarmInterface_ptr);

      /** Retrieve all of the Outstanding Alarms the agent */
      string getOutstandingAlarms();

      /** Retrieve the Inventory of all possible Alarms in the system */
      string getAlarmInventory();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      CliMenuAlarms(const CliMenuAlarms& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      CliMenuAlarms& operator= (const CliMenuAlarms& rhs);

      /** Pointer to the Alarms interface object. Here we use a pointer since
          we're passing in the reference from the main class that handles the Corba stuff */
      alarms::alarmInterface_ptr alarmInterface_ptr_;

      /** Default error string */
      string errorString_;

};

#endif
