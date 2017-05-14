/******************************************************************************
* 
* File name:   LoggerCommon.h 
* Subsystem:   Platform Services 
* Description: Common Logging routines used by both the Logger.h client side
*              and the LogProcessor server side.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_LOGGER_COMMON_H_
#define _PLAT_LOGGER_COMMON_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LogMessage.h"

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
 * LoggerCommon contains common logging routines used by both the 
 * Logger.h client side and the LogProcessor server side.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

// Extern the log subsystem names array
extern const char* logSubSystemName[];

class LoggerCommon
{
   public:

      /** Virtual Destructor */
      virtual ~LoggerCommon();

      /**
       * Convert raw time stamp to readable time string
       */
      static void convertTimeToString(long time, char* buf);

      /**
       * Convert raw time stamp to readable date string
       */
      static void convertTimeToDateString(long time, char* buf);

      /**
       * Format the logMessage into its viewable format for outputting to a file or to the screen
       */
      static void formatLogMessage(LogMessage* logMessage, char* buffer);

      /**
       * Provide for text wrapping functionality if necessary
       */
      static char* wrapFormattedLogText(char* buffer, char* wrapBuffer);

   protected:

   private:

      /** Constructor */
      LoggerCommon();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LoggerCommon(const LoggerCommon& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LoggerCommon& operator= (const LoggerCommon& rhs);

};

#endif
