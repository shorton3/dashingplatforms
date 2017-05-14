/******************************************************************************
* 
* File name:   PgConnection.h 
* Subsystem:   Platform Services 
* Description: PgConnection handles all of the database specific communication
*              for the Postgresql Database.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DATAMGR_PG_CONNECTION_H_
#define _PLAT_DATAMGR_PG_CONNECTION_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

// Postgresql client library header - NOTE: This gets copied to /usr/include
// upon Postgresql installation
#include <libpq-fe.h>

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DbConnection.h"

#include "platform/opm/OPMBase.h"

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
 * PgConnection handles all of the database specific communication
 * for the Postgresql Database. 
 * <p>
 * PgConnection is created when the ConnectionSet object creates an
 * OPM pool of PgConnection objects. The OPM then instantiates the objects.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Structure containing all of the params needed to initialize the Postgresql Connection */
struct PgInitParams
{
   string userid;
   string password;
   string connectionString;
   string dsn;
}; // end PgInitParams

class PgConnection : public DbConnection
{
   public:

      /** Virtual Destructor */
      virtual ~PgConnection();

      /**
       * Inherited from OPMBase.
       * Method will be called when the pool is created with these objects.
       * This method bootstraps the creation of each object. THIS OVERRIDES
       * the initialize method in OPMBASE (can't be both virtual and static)
       * @param initializer - Initializer integer or pointer to initialization
       *      data needed by the object. In this case, we pass in a pointer to 
       *      a PgInitParams struct.
       * @returns Pointer to the new OPMBase object.
       */
      static OPMBase* initialize(int initializer);

      /**
       * Inherited from OPMBase.
       * Pure Virtual method will be called before releasing the object
       * back into the OPM pool.
       */
      void clean();

      /**
       * Check the health of the database connection.
       * @returns OK if the connection is valid; otherwise ERROR
       */
      int checkConnectionHealth();

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT, or DELETE operation. For Postgresql, the
       * execution command string can contain multiple SQL statements separated by semicolons.
       * However, this should be used with care, especially around external interfaces,
       * since it could be vulnerable against SQL injection attacks. 'executeCommandWithParms'
       * will only allow a single SQL statement and is not vulnerable in this way.
       * @returns OK if the command was executed successfully; otherwise ERROR
       */
      int executeCommand(string& command);

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT, or DELETE operation. (To work with ostringstream.str().c_str()).
       * For Postgresql, the execution command string can contain multiple SQL
       * statements separated by semicolons. However, this should be used with care,
       * especially around external interfaces, since it could be vulnerable
       * against SQL injection attacks. 'executeCommandWithParms' will only allow
       * a single SQL statement and is not vulnerable in this way.
       * @returns OK if the command was executed successfully; otherwise ERROR
       */
      int executeCommand(const char* command);

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT or DELETE operation. For Postgresql, only
       * a single execution command can be contained in this static string. The 
       * number of Substitution parameters and a pointer array of their values 
       * should be passed in for the server to operate on.
       * @param numberParams number of passed in parameters in the char* array
       * @param paramValues specifies the actual values of the parameters (char* array)
       * @param paramLengths specifies the actual data lengths of binary-format parameters.
       *    It is ignored for null parameters and text-format parameters. The array
       *    pointer may be null when there are no binary parameters.
       * @param isBinary array for each param value to determine if the value is
       *    binary or textual (TRUE for binary / FALSE for text). The array pointer
       *    may be null when there are no binary parameters. 
       * @returns OK if the command was executed successfully; otherwise ERROR
       */
      int executeCommandWithParams(const char* command, int numberParams, 
            const char* const* paramValues, const int* paramLengths, const int* isBinary);

      /**
       * Begin a transaction on this connection. Autocommit is ENABLED for all
       * executed commands UNLESS beginTransaction has been called--that is, beginTransaction
       * disabled autocommit until either rollbackTransaction or commitTransaction
       * is called. Postgresql optionally supports the passing of the following 
       * isolation levels when the transaction is begun. Support for these can be added
       * to this interface in the future:
       * ISOLATION LEVEL { SERIALIZABLE | REPEATABLE READ | READ COMMITTED | READ UNCOMMITTED }
       *   READ WRITE | READ ONLY
       */
      int beginTransaction();

      /**
       * Rollback a transaction on this connection
       */
      int rollbackTransaction();

      /**
       * Commit the outstanding transaction on this connection
       */
      int commitTransaction();

      /**
       * Setup for a prepared statement command on this connection
       * @param statementName string name arbitrarily assigned to the statement
       * @param sqlCommand QUERY, UPDATE, INSERT, or DELETE sql operation.
       * @param numberParams number of passed in parameters in the char* array
       * @param paramTypeArray array of field types because the database cannot
       *    infer the field type formatting without it.
       * @returns OK if the preparation was successful; otherwise ERROR
       */
      int initializePreparedCommand(const char* statementName, const char* sqlCommand,
            int numberParams, const DataManagerFieldType* paramTypeArray);

      /**
       * Execute an SQL prepared statement command to the database using this connection.
       * This command can be a QUERY, UPDATE, INSERT, or DELETE operation.
       * @param statementName string name assigned to the statement when initializePreparedCommand 
       *    is called.
       * @param numberParams number of passed in parameters in the char* array
       * @param paramValues specifies the actual values of the parameters (char* array)
       * @param paramLengths specifies the actual data lengths of binary-format parameters.
       *    It is ignored for null parameters and text-format parameters. The array
       *    pointer may be null when there are no binary parameters.
       * @param isBinary array for each param value to determine if the value is
       *    binary or textual (TRUE for binary / FALSE for text). The array pointer
       *    may be null when there are no binary parameters.
       * @returns OK if the command was successfully executed; otherwise ERROR
       */
      int executePreparedCommand(const char* statementName, int numberParams,
            const char* const* paramValues, const int* paramLengths, const int* isBinary);

      /**
       * If an outstanding/open resultSet exists for this connection, then close it.
       * return ERROR for failure; otherwise OK
       */
      int closeCommandResult();

      /**
       * Return the number of columns in the current commandResult structure
       */
      int getColumnCount();

      /**
       * Return the specified column header name in the current commandResult
       */
      string getColumnName(int columnIndex);

      /**
       * Return the column index based on the specified column name
       */
      int getColumnIndex(const char* columnName);

      /**
       * Return the number of rows in the current commandResult
       */
      int getRowCount();

      /**
       * Return the value at the specified column/row location in the current 
       * commandResult. The value is returned as a string.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      const char* getStringValueAt(int rowIndex, int columnIndex);

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a 2-byte short type.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      short getShortValueAt(int rowIndex, int columnIndex);

      /** 
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a 4-byte integer type.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      int getIntValueAt(int rowIndex, int columnIndex);

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a boolean type.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      bool getBooleanValueAt(int rowIndex, int columnIndex);

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a string; this is because timestamps
       * are internally stored using 8 bytes (64 bits)!! We have to use the
       * Postgres library to convert them back into ascii.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      const char* getTimestampValueAt(int rowIndex, int columnIndex);

      /**
       * Returns true if the field Value is NULL
       */
      bool isValueNULL(int rowIndex, int columnIndex);

      /**
       * Returns the actual length of a field value in bytes
       */
      int getValueLength(int rowIndex, int columnIndex);

      /**
       * Close the socket connection to the database server 
       */
      void shutdown();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /** Constructor */
      PgConnection();

   private:

      /**
       * Perform the initial connection establishment to the Postgresql database.
       * @returns ERROR upon connection failure; otherwise OK
       */
      int establishConnection(string userid, string password, string dsn, string connectionString);

      /**
       * Re-Establish the connection with the database
       * @returns ERROR upone connection failure; otherwise OK
       */
      int reEstablishConnection();

      /**
       * Perform some analysis on the connection status / health. Log the results.
       * @returns the Postgresql specific status type (from /usr/include/libpq-fe.h)
       */
      int getConnectionStatus();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      PgConnection(const PgConnection& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      PgConnection& operator= (const PgConnection& rhs);

      /** Actual Postgresql database connection object from the Postgres library */
      PGconn* pgConn_;

      /** Command Result Set structure containing columns/rows from a query command */
      PGresult* pgResult_;

      /** Error string to return when requesting string results */
      const char* errorString_;

      /** Local Connection parameter: userid */
      string userid_;

      /** Local Connection parameter: password */
      string password_;

      /** Local Connection parameter: dsn */
      string dsn_;

      /** Local Connection parameter: connection string */
      string connectionString_;
};

#endif
