/******************************************************************************
*
* File name:   DataManagerTest.h
* Subsystem:   Platform Services
* Description: Implements the unit test code to exercise the Data Manager.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_DATA_MANAGER_TEST_H_
#define _PLAT_DATA_MANAGER_TEST_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/datamgr/DbConnectionHandle.h"

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
 * Test class exercises the Data Manager
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class DataManagerTest
{
   public:

      /** Default Constructor */
      DataManagerTest();

      /** Default Destructor */
      ~DataManagerTest();
      
      /** 
       * Initialize method. 
       * Initialize the DataManager and setup connections to the database.
       * @returns ERROR on failure; otherwise OK
       */
      int initialize();

      /** Clean up the connection / release */
      void cleanup();

      /** 
       * Test method: Basic
       * Perform Testing of operations on the database using execCommand: a simple query and results parsing
       * @returns ERROR on failure; otherwise OK
       */
      int runTest1();

      /**
       * Test method: Parameterized arguments to the sql statement
       * Perform testing of execCommandWithParams.
       * @returns ERROR on failure; otherwise OK
       */
      int runTest2();

      /**
       * Test method: Test boolean type specific results
       * Perform Testing of getBooleanValueAt on query results.
       * @returns ERROR on failure; otherwise OK
       */
      int runTest3();

      /**
       * Test method: Test a multistep transaction
       * @returns ERROR on failure; otherwise OK
       */
      int runTest4();

      /**
       * Test method: Test preparing and executing a Prepared statement
       * @returns ERROR on failure; otherwise OK
       */
      int runTest5();

   protected:

   private:

      /** Display query results with short values */
      int displayShortQueryResults();

      /** Test Connection */
      DbConnectionHandle* connectionHandle_;
};

#endif

