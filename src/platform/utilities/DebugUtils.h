/******************************************************************************
* 
* File name:   DebugUtils.h 
* Subsystem:   Platform Services 
* Description: Various debugging methods for use within the system by development
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DEBUG_UTILS_H_
#define _PLAT_DEBUG_UTILS_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

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
 * DebugUtils contains various debugging methods for use within the system by development
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class DebugUtils
{
   public:

      /** 
       * Capture a full system stack trace and output it to stdout
       */
      static void printStackTrace();

      /**
       * Capture a full system stack trace and output it to a file (/tmp/platform.stackTrace)
       * Use this version when malloc is broken and will not allow printStackTrace to work.
       */
      static void printStackTraceToFile();

   protected:

   private:

      /** Constructor */
      DebugUtils();

      /** Virtual Destructor */
      virtual ~DebugUtils();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      DebugUtils(const DebugUtils& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DebugUtils& operator= (const DebugUtils& rhs);

};

#endif
