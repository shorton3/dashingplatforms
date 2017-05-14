/******************************************************************************
* 
* File name:   ThreadTest.h 
* Subsystem:   Platform Services 
* Description: Unit Test for Thread Monitoring, Recovery and Restart
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_THREAD_TEST_H_
#define _PLAT_THREAD_TEST_H_

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
 * ThreadTest is the Unit Test for the Thread Monitoring, Recovery, and Restart
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ThreadTest
{
   public:

      /** Constructor */
      ThreadTest();

      /** Virtual Destructor */
      virtual ~ThreadTest();

      /** Initialization */
      void initialize();

      /** Perform Tests */
      static void performTests(void);

      /** Perform Restart Tests */
      static void performRestartTests(void);

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ThreadTest(const ThreadTest& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ThreadTest& operator= (const ThreadTest& rhs);

};

#endif
