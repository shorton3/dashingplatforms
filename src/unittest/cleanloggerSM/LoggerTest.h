/******************************************************************************
* 
* File name:   LoggerTest.h 
* Subsystem:   Platform Services 
* Description: Unit Test to clear the contents of shared memory
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_LOGGER_TEST_H_
#define _PLAT_LOGGER_TEST_H_

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
 * LoggerTest is a Unit Test utility for clearing the Logger Shared Memory Queue. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class LoggerTest
{
   public:

      /** Constructor */
      LoggerTest();

      /** Virtual Destructor */
      virtual ~LoggerTest();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LoggerTest(const LoggerTest& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LoggerTest& operator= (const LoggerTest& rhs);

};

#endif
