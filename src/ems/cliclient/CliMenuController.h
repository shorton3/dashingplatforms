/******************************************************************************
* 
* File name:   CliMenuController.h 
* Subsystem:   EMS
* Description: CliMenuController is responsible for menu parsing and display,
*              control of the user session, and invocation of menu items.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _EMS_CLI_MENU_CONTROLLER_H_
#define _EMS_CLI_MENU_CONTROLLER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "sessionI.h"
#include "CliMenuAlarms.h"
#include "CliMenuBaseActions.h"
#include "CliMenuStateActions.h"
#include "CliMenuPlatformConfig.h"

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
 * CliMenuController performs all CLI menu handling functions. 
 * <p>
 * CliMenuController is responsible for menu parsing and display,
 * control of the user session, and invocation of menu items.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

#define MAX_MAIN_USER_CHOICE             5 
#define MAX_STATE_TRANSITION_USER_CHOICE 4
#define MAX_HEALTH_REQUEST_USER_CHOICE   3
#define MAX_PLATFORM_CONFIG_USER_CHOICE  4
#define MAX_ALARMS_USER_CHOICE           4
#define QUIT_OPTION_INDEX                0
#define MAIN_MENU_OPTION_INDEX           1

class CliMenuController : public CliMenuBaseActions
{
   public:

      /** Constructor */
      CliMenuController();

      /** Virtual Destructor */
      virtual ~CliMenuController();

      /** Initialize the Menu and display the Main Menu Options */
      void initialize(session::Session_I_ptr session_I_ptr, 
                      platformConfig::platformConfig_I_ptr platformConfig_I_ptr,
                      alarms::alarmInterface_ptr alarmInterface_ptr);

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      CliMenuController(const CliMenuController& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      CliMenuController& operator= (const CliMenuController& rhs);

      /** Clear Screen method */
      void clearScreen();

      /** Quit the Menu method; Performs exit on the CLI */
      void quitMenu();

      /** Display the Main Menu Screen */
      void displayMainMenu();

      /** Display the State Transition Menu Screen */
      void displayStateTransitionMenu();

      /** Display the Health Menu Screen */
      void displayHealthRequestMenu();

      /** Display the Platform Config Menu Screen */
      void displayPlatformConfigMenu();

      /** Display the Alarms Menu Screen */
      void displayAlarmsMenu();

      /** Arrays of member function pointers for handling menu item actions **/
      CLIVOIDFUNCPTR mainCliMethodList_[MAX_MAIN_USER_CHOICE];
      CLIVOIDFUNCPTR stateTransitionCliMethodList_[MAX_STATE_TRANSITION_USER_CHOICE];
      CLIVOIDFUNCPTR healthRequestCliMethodList_[MAX_HEALTH_REQUEST_USER_CHOICE];
      CLIVOIDFUNCPTR platformConfigCliMethodList_[MAX_PLATFORM_CONFIG_USER_CHOICE];
      CLIVOIDFUNCPTR alarmsCliMethodList_[MAX_ALARMS_USER_CHOICE];

      /** Handler class for State Transition Actions */
      CliMenuStateActions cliMenuStateActions_;

      /** Handler class for Platform Config requests */
      CliMenuPlatformConfig cliMenuPlatformConfig_;

      /** Handler class for Alarm requests */
      CliMenuAlarms cliMenuAlarms_;

      /** Proxy methods for calling into one of the above Handler classes for menu invocation */
      void sendUnlockProxy();
      void sendLockProxy();
      void showLogLevels();
      void setLogLevel();
      void showOutstandingAlarms();
      void showInventoryAlarms();
};

#endif
