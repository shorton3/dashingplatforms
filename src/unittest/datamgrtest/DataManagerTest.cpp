/******************************************************************************
*
* File name:   DataManagerTest.cpp
* Subsystem:   Platform Services
* Description: Implements the unit test code to exercise the Data Manager.
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

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <sys/resource.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DataManagerTest.h"
#include "platform/datamgr/DataManager.h"

#include "platform/opm/OPM.h"

// Log Manager related includes.
#include "platform/logger/Logger.h"

// Common defines
#include "platform/common/Defines.h"
#include "platform/common/ConnectionSetNames.h"

#include "platform/utilities/SystemInfo.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Data Manager Test"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DataManagerTest::DataManagerTest() : connectionHandle_(NULL)
{
}//end Constructor


//-----------------------------------------------------------------------------
// Method Type: Destructor
// Description:
// Design:
//-----------------------------------------------------------------------------
DataManagerTest::~DataManagerTest()
{
}//end Destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize the DataManager and setup connections to the database
// Design:     
//-----------------------------------------------------------------------------
int DataManagerTest::initialize()
{
   // Initialize the static Data Manager
   if (DataManager::initialize("./DataManager.conf") == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Error initializing Data manager",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "Successfully initialized Data Manager",0,0,0,0,0,0);
   }//end else

   // Activate the Logger Connection Set
   if (DataManager::activateConnectionSet(TEST_CONNECTION) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Error activating Logger ConnectionSet",0,0,0,0,0,0);
      return ERROR;
   }//end if   
   else
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "Successfully activated Logger ConnectionSet",0,0,0,0,0,0);
   }//end else

   // Reserve a connection
   connectionHandle_ = DataManager::reserveConnection(TEST_CONNECTION);
   if (connectionHandle_ == NULL)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Error reserving connection",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "Successfully reserved Logger ConnectionSet Connection",0,0,0,0,0,0);
   }//end else

   // Just for fun here, add some User Defined Identifier to later identify the connection
   // when it comes out of the pool again
   string tmpString("someIdentifier");
   connectionHandle_->addUserDefinedIdentifier(tmpString);

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Release the connection
// Design: 
//-----------------------------------------------------------------------------
void DataManagerTest::cleanup()
{
   DataManager::releaseConnection(connectionHandle_);
}//end cleanup


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform Basic Test database operations
// Design:     
//-----------------------------------------------------------------------------
int DataManagerTest::runTest1()
{
   TRACELOG(DEBUGLOG, DATAMGRLOG, "######### Beginning Test for Basic Query functionality ############",0,0,0,0,0,0);

   // Run a test query against the LogLevels table
   string logLevelQuery = "Select Subsystem,LogLevel from platform.loglevels where NEID='" + 
      SystemInfo::getLocalNEID() + "'";

   // Check the health of the database connection
   if (connectionHandle_->checkConnectionHealth() == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Detected unhealthy database connection",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Execute the query against the database
   if (connectionHandle_->executeCommand(logLevelQuery) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Log Level query execution failed",0,0,0,0,0,0);
      return ERROR;
   }//end if

   return displayShortQueryResults();
}//end runTest1


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Parameterized arguments to the sql statement
// Design:
//-----------------------------------------------------------------------------
int DataManagerTest::runTest2()
{
   TRACELOG(DEBUGLOG, DATAMGRLOG, "######### Beginning Test for Parameterized Statements ############",0,0,0,0,0,0);

   // Run a test query against the LogLevels table
   const char* logLevelQuery = "Select LogLevel from platform.loglevels where NEID=$1 and SubSystem=$2;";
   // Prepare the query parameters
   short subSystemValue = (short)DATAMGRLOG;
   const char* subSystemPtr = (char*)&subSystemValue;
   const char* paramValues[]={ SystemInfo::getLocalNEID().c_str(), subSystemPtr }; 

   //cout << "Query for neid=" << SystemInfo::getLocalNEID().c_str() << " (" 
   //     << SystemInfo::getLocalNEID().length() << ") and subsystem=" << (short)(*subSystemPtr) << endl;

   // NEID length not needed since textual data
   const int paramLengths[]={ SystemInfo::getLocalNEID().length(), sizeof(short) }; 
   const int isBinary[]={ FALSE, TRUE }; /* text,binary */ 

   // NOTE!!! This query is not working as it returns NO ROWS. There is some problem with the encoding
   // of the short value. Check in /var/lib/pgsql/data/pg_log/<current file> for the log that it gives
   // (something about unexpected EOF)

   // Execute the query against the database
   if (connectionHandle_->executeCommandWithParams(logLevelQuery, 2, paramValues, paramLengths, isBinary) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Log Level query execution failed",0,0,0,0,0,0);
      return ERROR;
   }//end if

   return displayShortQueryResults();
}//end runTest2


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform Testing of getBooleanValueAt on query results (boolean type field results)
// Design:
//-----------------------------------------------------------------------------
int DataManagerTest::runTest3()
{
   TRACELOG(DEBUGLOG, DATAMGRLOG, "######### Beginning Test for Boolean Field Values ############",0,0,0,0,0,0);

   // Run a test query against the LogLevels table
   const char* logLevelQuery = "Select AlarmCode, AutoClearing from platform.AlarmsInventory;";

   // Execute the query against the database
   if (connectionHandle_->executeCommand(logLevelQuery) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Alarms Inventory query execution failed",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Parse the results of the query
   TRACELOG(DEBUGLOG, DATAMGRLOG, "Results for executed query:",0,0,0,0,0,0);

   int columnCount = connectionHandle_->getColumnCount();
   // Do some checks to see if the number of columns returned is zero! This indicates a schema error!
   if (columnCount == 0)
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "No columns returned in result set",0,0,0,0,0,0);
   }//end if
   else
   {
      // Don't assume the column ordering
      const int alarmCodeIndex = connectionHandle_->getColumnIndex("AlarmCode");
      const int autoClearingIndex = connectionHandle_->getColumnIndex("AutoClearing");

      TRACELOG(DEBUGLOG, DATAMGRLOG, "------------------",0,0,0,0,0,0);
      // Display the results in a Table-style format. FIRST, display all of the column headers in a log
      string rowBuffer("");
      for (int column = 0; column < columnCount; column++)
      {
         rowBuffer = rowBuffer + " " + connectionHandle_->getColumnName(column);
      }//end for
      STRACELOG(DEBUGLOG, DATAMGRLOG, rowBuffer.c_str());

      // Do some check to see if the number of rows returned is zero! This -POSSIBLY- indicates an empty database table!
      int rowCount = connectionHandle_->getRowCount();
      if (rowCount == 0)
      {
         TRACELOG(DEBUGLOG, DATAMGRLOG, "No rows returned in result set",0,0,0,0,0,0);
      }//end if
      else
      {
         // Build out a log that contains all of the columns in the row. NOTE: Here we make assumptions
         // about the order of the columns (assume its the same as in the above query). We really need to
         // use getColumnIndex(columnName) to determine the order!!
         for (int row = 0; row < rowCount; row++)
         {
            ostringstream ostr;
            ostr << "Row" << row << "  ";
            for (int column = 0; column < columnCount; column++)
            {
               if (column == alarmCodeIndex)
               {
                  ostr << "(" << connectionHandle_->getShortValueAt(row, column) << ")  ";
               }//end if
               else if (column == autoClearingIndex)
               {
                  ostr << "(" << connectionHandle_->getBooleanValueAt(row, column) << ")  ";
               }//end else if
            }//end for
            ostr << ends;
            STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         }//end else
      }//end for
      TRACELOG(DEBUGLOG, DATAMGRLOG, "------------------",0,0,0,0,0,0);
   }//end else

   // Deallocate the resources for the query
   connectionHandle_->closeCommandResult();

   return OK;
}//end runTest3


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Test method: Test a multistep transaction
// Design:
//-----------------------------------------------------------------------------
int DataManagerTest::runTest4()
{
   TRACELOG(DEBUGLOG, DATAMGRLOG, "######### Beginning Test for Transactions ############",0,0,0,0,0,0);

   // Begin a Transaction
   connectionHandle_->beginTransaction();

   // Run a successful update on the LogLevels table
   string logLevelUpdate = "Update platform.LogLevels set logLevel=4 where NEID='" + 
       SystemInfo::getLocalNEID() + "' and SubSystem=2;";
                                                                                                                              
   // Execute the update against the database
   if (connectionHandle_->executeCommand(logLevelUpdate) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "LogLevels update execution failed. SHOULD NOT SEE THIS LOG",0,0,0,0,0,0);
      connectionHandle_->rollbackTransaction();
      return ERROR;
   }//end if

   // Now do something dumb to cause us to rollback
   logLevelUpdate = "Update platform.LogLevels set logLevel='dumb' where NEID=''" + 
      SystemInfo::getLocalNEID() + "' and SubSystem=3;";

   // Execute the update against the database
   if (connectionHandle_->executeCommand(logLevelUpdate) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "LogLevels update execution failed. THIS IS AN INTENDED FAILURE",0,0,0,0,0,0);
      connectionHandle_->rollbackTransaction();
   }//end if

   // Begin another transaction
   connectionHandle_->beginTransaction();

   // Now that we have rolled back, try to update another tuple
   logLevelUpdate = "Update platform.LogLevels set logLevel=3 where NEID='" + 
      SystemInfo::getLocalNEID() + "' and SubSystem=3;";

   // Execute the update against the database
   if (connectionHandle_->executeCommand(logLevelUpdate) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "LogLevels update execution failed. SHOULD NOT SEE THIS LOG",0,0,0,0,0,0);
      connectionHandle_->rollbackTransaction();
      return ERROR;
   }//end if

   // Now commit the transaction -- only the previous/last command should be persisted
   connectionHandle_->commitTransaction();

   // Run test1 again to display the loglevels table contents
   TRACELOG(DEBUGLOG, DATAMGRLOG, "Running test1 again to display LogLevels table contents",0,0,0,0,0,0);
   return runTest1();
}//end runTest4


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Test method: Test preparing and executing a Prepared statement
// Design:
//-----------------------------------------------------------------------------
int DataManagerTest::runTest5()
{
   TRACELOG(DEBUGLOG, DATAMGRLOG, "######### Beginning Test for Prepared Statements ############",0,0,0,0,0,0);

   const char* statementName = "TEST_PREPARED_STATEMENT";

   // Create the prepared statement and prepare it with the database
   DataManagerFieldType type1 = CHAR_FIELD_TYPE;
   DataManagerFieldType type2 = INT2_FIELD_TYPE;
   const DataManagerFieldType paramTypeArray[] = { type1, type2 };
   if (connectionHandle_->initializePreparedCommand(statementName,
       "Select LogLevel from platform.loglevels where NEID=$1 and SubSystem=$2;", 2, paramTypeArray) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Initialization of prepared statement failed",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Prepare the query parameters
   short subSystemValue = (short)DATAMGRLOG;
   const char* subSystemPtr = (char*)&subSystemValue;
   const char* paramValues[]={ SystemInfo::getLocalNEID().c_str(), subSystemPtr };
   const int paramLengths[]={ SystemInfo::getLocalNEID().length(), sizeof(short) };
   const int isBinary[]={ FALSE, TRUE }; /* text,binary */

   // NOTE!!! This query is not working as it returns NO ROWS. There is some problem with the encoding
   // of the short value. Check in /var/lib/pgsql/data/pg_log/<current file> for the log that it gives
   // (something about unexpected EOF)

   // Execute the prepared statement with the appropriate parameters
   if (connectionHandle_->executePreparedCommand(statementName, 2, paramValues, paramLengths, isBinary) == ERROR)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Log Level prepared statement query execution failed",0,0,0,0,0,0);
      return ERROR;
   }//end if

   return displayShortQueryResults();
}//end runTest5


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Display query results
// Design:
//-----------------------------------------------------------------------------
int DataManagerTest::displayShortQueryResults()
{
   // Parse the results of the query
   TRACELOG(DEBUGLOG, DATAMGRLOG, "Results for executed query:",0,0,0,0,0,0);

   int columnCount = connectionHandle_->getColumnCount();
   // Do some checks to see if the number of columns returned is zero! This indicates a schema error!
   if (columnCount == 0)
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "No columns returned in result set",0,0,0,0,0,0);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "------------------",0,0,0,0,0,0);
      // Display the results in a Table-style format. FIRST, display all of the column headers in a log
      string rowBuffer("");
      for (int column = 0; column < columnCount; column++)
      {
         rowBuffer = rowBuffer + " " + connectionHandle_->getColumnName(column);
      }//end for
      STRACELOG(DEBUGLOG, DATAMGRLOG, rowBuffer.c_str());

      // Do some check to see if the number of rows returned is zero! This -POSSIBLY- indicates an empty database table!
      int rowCount = connectionHandle_->getRowCount();
      if (rowCount == 0)
      {
         TRACELOG(DEBUGLOG, DATAMGRLOG, "No rows returned in result set",0,0,0,0,0,0);
      }//end if
      else
      {
         // Build out a log that contains all of the columns in the row. NOTE: Here we make assumptions
         // about the order of the columns (assume its the same as in the above query). We really need to
         // use getColumnIndex(columnName) to determine the order!!
         for (int row = 0; row < rowCount; row++)
         {
            ostringstream ostr;
            ostr << "Row" << row << "  ";
            for (int column = 0; column < columnCount; column++)
            {
               ostr << "(" << connectionHandle_->getShortValueAt(row, column) << ")  ";
            }//end for
            ostr << ends;
            STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         }//end for
      }//end else
      TRACELOG(DEBUGLOG, DATAMGRLOG, "------------------",0,0,0,0,0,0);
   }//end else

   // Deallocate the resources for the query
   connectionHandle_->closeCommandResult();

   return OK;
}//end displayShortQueryResults


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: main function for test binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   // do some dummy stuff on bufPtr to prevent compiler warning - this code will be
   // removed by the optimizer when it runs
   int tmpInt __attribute__ ((unused)) = argc;
   char** tmpChar __attribute__ ((unused)) = argv;

   // Turn of OS limits
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // Initialize the Logger with local-only output
   Logger::getInstance()->initialize(true);

   // Turn on OPM Subsystem log levels
   Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
   Logger::setSubsystemLogLevel(DATAMGRLOG, DEVELOPERLOG);
 
   // Initialize the OPM
   OPM::initialize();

   // Create our Test Object
   DataManagerTest* dataMgr = new DataManagerTest();

   // Initialize everything
   if (dataMgr->initialize() == ERROR)
   {
      return ERROR;
   }//end if

   // Perform the tests
   if (dataMgr->runTest1() == ERROR)
   {
      return ERROR;
   }//end if

   if (dataMgr->runTest2() == ERROR)
   {
      return ERROR;
   }//end if

   if (dataMgr->runTest3() == ERROR)
   {
      return ERROR;
   }//end if

   if (dataMgr->runTest4() == ERROR)
   {
      return ERROR;
   }//end if

   if (dataMgr->runTest5() == ERROR)
   {
      return ERROR;
   }//end if

   // Release the connection 
   dataMgr->cleanup();

   return OK;
}//end main
