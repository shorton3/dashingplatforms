/******************************************************************************
*
* File name:   LoggerSMConfigValues.cpp
* Subsystem:   Platform Services
* Description: LoggerSMConfigValues class encapsulates the configuration parameters
*              needed by the LogProcessor and the Logger clients.
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
#include <cstring>
#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LoggerSMConfigValues.h"

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
LoggerSMConfigValues::LoggerSMConfigValues()
{
   //cout << "LoggerSMConfigValues Constructor" << endl;

   // Loop through all of the subsystem Ids and set the log level to DEVELOPER (for now)
   reset();
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Copy Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
LoggerSMConfigValues::LoggerSMConfigValues( const LoggerSMConfigValues& rhs )
{
   //cout << "LoggerSMConfigValues Copy Constructor" << endl;

   ACE_OS::memcpy(logLevels, rhs.logLevels, sizeof(logLevels));
   logLevelsPI = logLevels;
}//end Copy Constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LoggerSMConfigValues::~LoggerSMConfigValues()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reset the data members
// Design:
//-----------------------------------------------------------------------------
void LoggerSMConfigValues::reset()
{
   //cout << "LoggerSMConfigValues Reset" << endl;

   // Loop through all of the subsystem Ids and set the log level to DEVELOPER (for now)
   for (int i = 0; i <= MAX_LOG_SUBSYSTEM; i++)
   {
      logLevels[i] = (int)DEVELOPERLOG;
   }//end for

   // Assign the logLevels array to the position independent pointer
   logLevelsPI = logLevels;
}//end reset


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded assignment operator
// Design:
//-----------------------------------------------------------------------------
LoggerSMConfigValues& LoggerSMConfigValues::operator= ( const LoggerSMConfigValues& rhs )
{
   //cout << "LoggerSMConfigValues Assignment operator" << endl;
 
   if (this != &rhs)
   {
      ACE_OS::memcpy(logLevels, rhs.logLevels, sizeof(logLevels));
      logLevelsPI = logLevels;
   }//end if
   return *this;
}//end assignment operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Equality Operator
// Design:
//-----------------------------------------------------------------------------
bool LoggerSMConfigValues::operator== (const LoggerSMConfigValues& rhs)
{
   //cout << "LoggerSMConfigValues Equality operator" << endl;

   // Loop through all of the subsystem Ids and check equivalence
   for (int i = 0; i <= MAX_LOG_SUBSYSTEM; i++)
   {
      if (logLevels[i] != rhs.logLevels[i])
      {
         return false;
      }//end if
   }//end for
   return true;
}//end equality operator


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

