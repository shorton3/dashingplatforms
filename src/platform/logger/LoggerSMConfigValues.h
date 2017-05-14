/******************************************************************************
* 
* File name:   LoggerSMConfigValues.h 
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

#ifndef _PLAT_LOGGER_SM_CONFIG_VALUES_H_
#define _PLAT_LOGGER_SM_CONFIG_VALUES_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Based_Pointer_T.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/common/Defines.h"

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
 * LoggerSMConfigValues class encapsulates the configuration parameters
 * needed by the LogProcessor and the Logger clients.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Shared Memory Initialization parameters */
#define LOG_CONFIG_NAME          "LogConfig"
#define LOG_CONFIG_MUTEXNAME     "LogConfigMutex"

struct LoggerSMConfigValues
{
   /** 
    * Two dimensional static array that maps Subsystem Id to the LogLevel
    * as defined in platform/common/Defines.h
    */
   int logLevels[MAX_LOG_SUBSYSTEM];

   /** Position Independent ACE Pointer to the log levels map. NOTE: there is no allocated memory here */
   ACE_Based_Pointer_Basic<int> logLevelsPI;

   /** Default Constructor */
   LoggerSMConfigValues();

   /** Copy Constructor */
   LoggerSMConfigValues( const LoggerSMConfigValues& rhs );

   /** Destructor */
   ~LoggerSMConfigValues(void);

   /** Reset data members */
   void reset(void);

   /** Overloaded assignment operator */
   LoggerSMConfigValues& operator= ( const LoggerSMConfigValues& rhs );

   /** Overloaded equality operator */
   bool operator== (const LoggerSMConfigValues& rhs);

};//end LoggerSMConfigValues structure

#endif
