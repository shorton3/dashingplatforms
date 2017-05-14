/******************************************************************************
*
* File name:   CliMenuController.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliMenuController.h"

#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
CliMenuController::CliMenuController()
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
CliMenuController::~CliMenuController()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize the Menu and display the Main Menu Options
// Design:      Add the Handlers for NEW menu items here
//-----------------------------------------------------------------------------
void CliMenuController::initialize(session::Session_I_ptr session_I_ptr,
                                   platformConfig::platformConfig_I_ptr platformConfig_I_ptr,
                                   alarms::alarmInterface_ptr alarmInterface_ptr)
{
   // Initialize the static handler classes
   cliMenuStateActions_.initialize(session_I_ptr);
//   cliMenuHealthActions_.initialize(session_I_ptr);
   cliMenuPlatformConfig_.initialize(platformConfig_I_ptr);
   cliMenuAlarms_.initialize(alarmInterface_ptr);

   // Populate the CLI Menu Array with the support methods. NOTE!! Because we
   // need access to an instance pointer to these methods, we have to create a 
   // small method to call into the Handler Class (so that we can setup a function
   // pointer to it).
   mainCliMethodList_[0] = (CLIVOIDFUNCPTR)&CliMenuController::quitMenu;
   mainCliMethodList_[1] = (CLIVOIDFUNCPTR)&CliMenuController::displayStateTransitionMenu;
   mainCliMethodList_[2] = (CLIVOIDFUNCPTR)&CliMenuController::displayHealthRequestMenu;
   mainCliMethodList_[3] = (CLIVOIDFUNCPTR)&CliMenuController::displayPlatformConfigMenu;
   mainCliMethodList_[4] = (CLIVOIDFUNCPTR)&CliMenuController::displayAlarmsMenu;

   stateTransitionCliMethodList_[0] = (CLIVOIDFUNCPTR)&CliMenuController::quitMenu;
   stateTransitionCliMethodList_[1] = (CLIVOIDFUNCPTR)&CliMenuController::displayMainMenu;
   stateTransitionCliMethodList_[2] = (CLIVOIDFUNCPTR)&CliMenuController::sendUnlockProxy;
   stateTransitionCliMethodList_[3] = (CLIVOIDFUNCPTR)&CliMenuController::sendLockProxy;

   healthRequestCliMethodList_[0] = (CLIVOIDFUNCPTR)&CliMenuController::quitMenu;
   healthRequestCliMethodList_[1] = (CLIVOIDFUNCPTR)&CliMenuController::displayMainMenu;
// OPTIONS MISSING HERE!!! We NEED THE FOLLOWING:
//   healthRequestCliMethodList_[2] = (CLIVOIDFUNCPTR)&CliMenuHealthActions::queryCard;

   platformConfigCliMethodList_[0] = (CLIVOIDFUNCPTR)&CliMenuController::quitMenu;
   platformConfigCliMethodList_[1] = (CLIVOIDFUNCPTR)&CliMenuController::displayMainMenu;
   platformConfigCliMethodList_[2] = (CLIVOIDFUNCPTR)&CliMenuController::showLogLevels;
   platformConfigCliMethodList_[3] = (CLIVOIDFUNCPTR)&CliMenuController::setLogLevel;

   alarmsCliMethodList_[0] = (CLIVOIDFUNCPTR)&CliMenuController::quitMenu;
   alarmsCliMethodList_[1] = (CLIVOIDFUNCPTR)&CliMenuController::displayMainMenu;
   alarmsCliMethodList_[2] = (CLIVOIDFUNCPTR)&CliMenuController::showOutstandingAlarms;
   alarmsCliMethodList_[3] = (CLIVOIDFUNCPTR)&CliMenuController::showInventoryAlarms;

   // Start the Menu Display Loop - we stay in here forever until the user selects Quit
   displayMainMenu();

}//end initialize


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Clear Screen method
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::clearScreen()
{
   //sleep(1);
   printf("\33[2J");
   //sleep(1);
}//end clearScreen


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Quit the Menu method; Performs exit on the CLI
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::quitMenu()
{
   exit(OK);
}//end quitMenu


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Display the Main Menu Screen
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::displayMainMenu()
{
   int userChoice = -1;
   char charHolder;
   //create a local static variable to track just the first time we run it (we should not clear screen)
   static bool isFirstTime = true;

   while (userChoice != QUIT_OPTION_INDEX)
   {
      // Clear the screen only on subsequent times we run this (so we see initialization messages)
      if (!isFirstTime)
         clearScreen();
      else
         isFirstTime = false;

      printf("\nEMS CLI Client\n");
      printf(  "---------------------------------\n");

      printf("\nMain menu:\n");
      printf(  "----------\n");

      printf("0. Quit\n");
      printf("1. State Transition Commands\n");
      printf("2. Health Query Requests\n");
      printf("3. Platform Config\n");
      printf("4. Alarms\n");

      printf("\nEnter your choice: ");
      scanf("%d", &userChoice);

      if (userChoice == QUIT_OPTION_INDEX)
      {
         break;
      }//end if

      if ((userChoice > 0) && (userChoice < MAX_MAIN_USER_CHOICE))
      {
         clearScreen();
         (this->*mainCliMethodList_[userChoice])();

         printf("\nPlease press <Enter> to continue.....");
         scanf("%c", &charHolder);
      }//end if
   }//end while
}//end displayMainMenu


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Display the State Transition Menu Screen
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::displayStateTransitionMenu()
{
   int userChoice = -1;
   char charHolder;

   while ( (userChoice != MAIN_MENU_OPTION_INDEX) || (userChoice != QUIT_OPTION_INDEX) )
   {
      clearScreen();

      printf("\nState Transition Commands Menu:\n");
      printf(  "------------------------------\n");

      printf("0. Quit\n");
      printf("1. Return to Main Menu\n");
      printf("2. Send Unlock Message\n");
      printf("3. Send Lock Message\n");

      printf("\nEnter your choice: ");
      scanf("%d", &userChoice);

      if (userChoice == QUIT_OPTION_INDEX)
      {
         exit(OK);
      }//end if

      if (userChoice == MAIN_MENU_OPTION_INDEX)
      {
         break;
      }//end if

      if ( (userChoice > 0) && (userChoice < MAX_STATE_TRANSITION_USER_CHOICE) )
      {
         //clearScreen();
         (this->*stateTransitionCliMethodList_[userChoice])();

         printf("\nPlease press <Enter> to continue.....");
         scanf("%c", &charHolder);
      }//end if
   }//end while
}//end displayStateTransitionMenu


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Display the Health Request Menu Screen
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::displayHealthRequestMenu()
{
   int userChoice = -1;
   char charHolder;

   while ( (userChoice != MAIN_MENU_OPTION_INDEX) || (userChoice != QUIT_OPTION_INDEX) )
   {
      clearScreen();

      printf("\nHealth Request Menu:\n");
      printf(  "------------------------------\n");

      printf("0. Quit\n");
      printf("1. Return to Main Menu\n");
      printf("2. Query Card Health\n");

      printf("\nEnter your choice: ");
      scanf("%d", &userChoice);

      if (userChoice == QUIT_OPTION_INDEX)
      {
         exit(OK);
      }//end if

      if (userChoice == MAIN_MENU_OPTION_INDEX)
      {
         break;
      }//end if

      if ( (userChoice > 0) && (userChoice < MAX_HEALTH_REQUEST_USER_CHOICE) )
      {
         //clearScreen();
         (this->*healthRequestCliMethodList_[userChoice])();

         printf("\nPlease press <Enter> to continue.....");
         scanf("%c", &charHolder);
      }//end if
   }//end while
}//end displayHealthRequestMenu


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Display the Platform Configuration Menu Screen
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::displayPlatformConfigMenu()
{
   int userChoice = -1;
   char charHolder;

   while ( (userChoice != MAIN_MENU_OPTION_INDEX) || (userChoice != QUIT_OPTION_INDEX) )
   {
      clearScreen();

      printf("\nPlatform Config Menu:\n");
      printf(  "------------------------------\n");

      printf("0. Quit\n");
      printf("1. Return to Main Menu\n");
      printf("2. Show Log Levels\n");
      printf("3. Set Log Level\n");

      printf("\nEnter your choice: ");
      scanf("%d", &userChoice);

      if (userChoice == QUIT_OPTION_INDEX)
      {
         exit(OK);
      }//end if

      if (userChoice == MAIN_MENU_OPTION_INDEX)
      {
         break;
      }//end if

      if ( (userChoice > 0) && (userChoice < MAX_PLATFORM_CONFIG_USER_CHOICE) )
      {
         //clearScreen();
         (this->*platformConfigCliMethodList_[userChoice])();

         printf("\nPlease press <Enter> to continue.....");
         scanf("%c", &charHolder);
      }//end if
   }//end while
}//end displayPlatformConfigMenu


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Display the Alarms Menu Screen
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::displayAlarmsMenu()
{
   int userChoice = -1;
   char charHolder;

   while ( (userChoice != MAIN_MENU_OPTION_INDEX) || (userChoice != QUIT_OPTION_INDEX) )
   {
      clearScreen();

      printf("\nAlarms Menu:\n");
      printf(  "------------------------------\n");

      printf("0. Quit\n");
      printf("1. Return to Main Menu\n");
      printf("2. Show Outstanding Alarms\n");
      printf("3. Show Inventory of Alarms\n");

      printf("\nEnter your choice: ");
      scanf("%d", &userChoice);

      if (userChoice == QUIT_OPTION_INDEX)
      {
         exit(OK);
      }//end if

      if (userChoice == MAIN_MENU_OPTION_INDEX)
      {
         break;
      }//end if

      if ( (userChoice > 0) && (userChoice < MAX_ALARMS_USER_CHOICE) )
      {
         //clearScreen();
         (this->*alarmsCliMethodList_[userChoice])();

         printf("\nPlease press <Enter> to continue.....");
         scanf("%c", &charHolder);
      }//end if
   }//end while
}//end displayAlarmsMenu


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Proxy Method for calling into one of the handler classes
//              for menu invocation.
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::sendUnlockProxy()
{
   cliMenuStateActions_.sendUnlock();
}//end sendUnlockProxy


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Proxy Method for calling into one of the handler classes
//              for menu invocation.
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::sendLockProxy()
{
   cliMenuStateActions_.sendLock();
}//end sendLockProxy


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Proxy Method for calling into one of the handler classes
//              for menu invocation.
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::showLogLevels()
{
   string logLevelString = cliMenuPlatformConfig_.getLogLevels();

   printf("\nDisplaying log levels by subsystem:\n%s", logLevelString.c_str());
   char charHolder;
   scanf("%c", &charHolder);
}//end showLogLevels


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Proxy Method for calling into one of the handler classes
//              for menu invocation.
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::setLogLevel()
{
   int subsystemChoice = -1;
   while (1)
   {
      printf("\nEnter the subsystem numeric Id: ");
      scanf("%d", &subsystemChoice);
      if ((subsystemChoice >= ALL_LOG_SUBSYSTEMS) && (subsystemChoice < MAX_LOG_SUBSYSTEM))
      {
         break;
      }//end if
   }//end while

   int severityChoice = -1;
   while (1)
   {
      printf("\nEnter the severity level for subsystem %d (ERROR=1 WARNING=2 INFO=3 DEBUG=4 DEVELOPER=5): ", subsystemChoice);
      scanf("%d", &severityChoice);
      if ((severityChoice >= ERRORLOG) && (severityChoice < LAST_LOG_SEVERITY))
      {
         break;
      }//end if
   }//end while 
   
   cliMenuPlatformConfig_.setLogLevel(subsystemChoice, severityChoice);
}//end setLogLevel


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Proxy Method for calling into one of the handler classes
//              for menu invocation.
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::showOutstandingAlarms()
{
   string alarmsString = cliMenuAlarms_.getOutstandingAlarms();

   printf("\nDisplaying Outstanding Alarms:\n%s", alarmsString.c_str());
   char charHolder;
   scanf("%c", &charHolder);
}//end showOutstandingAlarms


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Proxy Method for calling into one of the handler classes
//              for menu invocation.
// Design:
//-----------------------------------------------------------------------------
void CliMenuController::showInventoryAlarms()
{
   string alarmsString = cliMenuAlarms_.getAlarmInventory();

   printf("\nDisplaying Inventory of all Alarms:\n%s", alarmsString.c_str());
   char charHolder;
   scanf("%c", &charHolder);
}//end showInventoryAlarms

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

