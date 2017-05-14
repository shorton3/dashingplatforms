/******************************************************************************
* 
* File name:   DbConnection.h 
* Subsystem:   Platform Services 
* Description: Provides an abstract base class for deriving all database-specific
*              connection classes from.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DATAMGR_DB_CONNECTION_H_
#define _PLAT_DATAMGR_DB_CONNECTION_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/opm/OPM.h"

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
 * DbConnection provides an abstract base class for deriving all database-specific
 * connection classes from. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class DbConnection : public OPMBase
{
   public:

      /** Virtual Destructor */
      virtual ~DbConnection();

      /**
       * Allows the Application Developer to 'tag' a particular connection
       * (through this connection handle) with some user-defined identification
       * data. This data tag will be preserved even if the connection it checked
       * back into the pool (released) and then reserved again. Thus, the developer
       * can identify which connection was used for which previous tasks.
       */
      void addUserDefinedIdentifier(string& identifier);

      /**
       * Returns the user-defined identifier data
       * @returns empty string if no user-defined data has been previously set
       */
      string& getUserDefinedIdentifier();

      /**
       * Clears the user-defined data and resets the internal string to empty
       */
      void clearUserDefinedIdentifier();

      /**
       * Check the health of the database connection.
       * @returns OK if the connection is valid; otherwise ERROR
       */
      virtual int checkConnectionHealth() = 0;

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT, or DELETE operation.
       * @returns OK if the command was executed successfully; otherwise ERROR
       */
      virtual int executeCommand(string& command) = 0;

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT, or DELETE operation. (To work with ostringstream)
       * @returns OK if the command was executed successfully; otherwise ERROR
       */
      virtual int executeCommand(const char* command) = 0;

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT or DELETE operation. 
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
      virtual int executeCommandWithParams(const char* command, int numberParams,
            const char* const* paramValues, const int* paramLengths, const int* isBinary) = 0;

      /**
       * Begin a transaction on this connection. Autocommit is ENABLED for all
       * executed commands UNLESS beginTransaction has been called--that is, beginTransaction
       * disabled autocommit until either rollbackTransaction or commitTransaction
       * is called.
       */
      virtual int beginTransaction() = 0;

      /**
       * Rollback a transaction on this connection
       */
      virtual int rollbackTransaction() = 0;

      /**
       * Commit the outstanding transaction on this connection
       */
      virtual int commitTransaction() = 0;

      /**
       * Setup for a prepared statement command on this connection
       * @param statementName string name arbitrarily assigned to the statement
       * @param sqlCommand QUERY, UPDATE, INSERT, or DELETE sql operation.
       * @param numberParams number of passed in parameters in the char* array
       * @param paramTypeArray array of field types because the database cannot
       *    infer the field type formatting without it.
       * @returns OK if the preparation was successful; otherwise ERROR
       */
      virtual int initializePreparedCommand(const char* statementName, const char* sqlCommand,
            int numberParams, const DataManagerFieldType* paramTypeArray) = 0;

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
      virtual int executePreparedCommand(const char* statementName, int numberParams,
            const char* const* paramValues, const int* paramLengths, const int* isBinary) = 0;

      /**
       * If an outstanding/open command resultSet exists for this connection, then close it.
       */
      virtual int closeCommandResult() = 0;

      /**
       * Return the number of columns in the current commandResult structure
       */
      virtual int getColumnCount() = 0;

      /**
       * Return the specified column header name in the current commandResult
       */
      virtual string getColumnName(int columnIndex) = 0;

      /**
       * Return the column index based on the specified column name
       */
      virtual int getColumnIndex(const char* columnName) = 0;

      /**
       * Return the number of rows in the current commandResult
       */
      virtual int getRowCount() = 0;

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value is returned as a string.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      virtual const char* getStringValueAt(int rowIndex, int columnIndex) = 0;

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a 2-byte short type.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      virtual short getShortValueAt(int rowIndex, int columnIndex) = 0;

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a 4-byte integer type.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      virtual int getIntValueAt(int rowIndex, int columnIndex) = 0;

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a boolean type.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      virtual bool getBooleanValueAt(int rowIndex, int columnIndex) = 0;

      /**
       * Return the value at the specified column/row location in the current
       * commandResult. The value returned will be a string.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      virtual const char* getTimestampValueAt(int rowIndex, int columnIndex) = 0;

      /**
       * Returns true if the field Value is NULL
       */
      virtual bool isValueNULL(int rowIndex, int columnIndex) = 0;

      /**
       * Returns the actual length of a field value in bytes
       */
      virtual int getValueLength(int rowIndex, int columnIndex) = 0;      

      /**
       * Close the socket connection to the database server
       */
      virtual void shutdown() = 0;

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /** Constructor */
      DbConnection();

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      DbConnection(const DbConnection& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DbConnection& operator= (const DbConnection& rhs);

      /** User defined Identifier data string */
      string userDefinedIdentifierString_;

};

#endif
