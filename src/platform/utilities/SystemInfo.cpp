/******************************************************************************
*
* File name:   SystemInfo.cpp
* Subsystem:   Platform Services
* Description: Provides static access to system wide attributes such as NEID,
*              Shelf Number, Slot Number, etc.
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

#include <cctype>
#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "SystemInfo.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Flag to determine if we already retrieved the Local NEID from the environment
bool SystemInfo::isNEIDInitialized_ = false;

// Static Local NEID string
string SystemInfo::localNEIDStr_ = "000000000";

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return the local card's NEID
// Design:     
//-----------------------------------------------------------------------------
const string& SystemInfo::getLocalNEID()
{
   if (!isNEIDInitialized_)
   {
      isNEIDInitialized_ = true;
      char* localNEID = getenv("LOCAL_NEID");

      // Check to make sure that the LOCAL_NEID env variable is set
      if (localNEID == NULL)
      {
         cout << endl << "SystemInfo reports that LOCAL_NEID environment variable is not set" << endl;
         return localNEIDStr_;
      }//end if

      if (strlen(localNEID) == 9)
      {
         for (int i = 0; i < 9; i++)
         {
            if (!isdigit(localNEID[i]))
            {
               // Invalid NEID (must be 9 digits) - Abort with bad number
               return localNEIDStr_;
            }//end if
         }//end for 
         // NEID appears to be good, so assign and return it
         localNEIDStr_ = localNEID;
      }//end if
   }//end if
   return localNEIDStr_;   
}//end getLocalNEID


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return the local card's shelf number 
// Design:     
//-----------------------------------------------------------------------------
unsigned int SystemInfo::getShelfNumber()
{
//TODO
   return 1;
}//end getShelfNumber


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return the local card's slot number in the shelf
// Design:     
//-----------------------------------------------------------------------------
unsigned int SystemInfo::getSlotNumber()
{
//TODO
   return 1;
}//end getSlotNumber


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

