/******************************************************************************
* 
* File name:   DbConnectionHandle.h 
* Subsystem:   Platform Services 
* Description: This class provides a wrapper around the 'real' DbConnection
*              object. The strategy here is to prevent the application designer
*              user from gaining access to the DbConnection object and doing
*              something they shouldn't.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DATAMGR_CONNECTION_HANDLER_H_
#define _PLAT_DATAMGR_CONNECTION_HANDLER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Thread.h>

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
 * DbConnectionHandle class provides a wrapper around the 'real' DbConnection
 * object. 
 * <p>
 * The strategy here is to prevent the application designer
 * user from gaining access to the DbConnection object and doing
 * something they shouldn't.
 * <p>
 * Additionally, DbConnectionHandle stores the Thread Id of the thread that
 * reserved the connection. For each operation, it checks to see if the thread
 * that is performing the operation is the same thread that reserved the connection.
 * If it is determined to be different, this can indicate that the application is
 * using a stale pointer to a connection that has already been release back into
 * the pool. The database driver API is 'mostly' thread safe. However, the architecture
 * of the driver is such that it is intended for use by a single thread--this means
 * that once a query has been performed on the connection and the result set has been
 * returned to the client side via the connection, another query should not be issued
 * until the result set has been completely analyzed (all processing on it complete),
 * and the result set has been closed.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class DbConnectionHandle : public OPMBase
{
   /** DataManager has to be a friend to allow calling getConnectionSetName
       (Required unless we give application designer access to these methods) */
   friend class DataManager;

   /** ConnectionSet is a friend to allow access to associate/dissociate connections
       (Required unless we give application designer access to these methods) */
   friend class ConnectionSet;

   public:

      /** Constructor */
      DbConnectionHandle();

      /** Virtual Destructor */
      virtual ~DbConnectionHandle();

      /**
       * Inherited from OPMBase.
       * Method will be called when the pool is created with these objects.
       * This method bootstraps the creation of each object. THIS OVERRIDES
       * the initialize method in OPMBASE (can't be both virtual and static)
       * @param initializer - Initializer integer or pointer to initialization
       *      data needed by the object. 
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
      int checkConnectionHealth();

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT, or DELETE operation.
       * @returns OK if the command was executed successfully; otherwise ERROR
       */
      int executeCommand(string& command);

      /**
       * Execute an SQL command to the database using this connection. This command
       * can be a QUERY, UPDATE, INSERT, or DELETE operation.
       * @returns OK if the command was executed successfully; otherwise ERROR
       */
      int executeCommand(const char* command);

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
      int executeCommandWithParams(const char* command, int numberParams,
            const char* const* paramValues, const int* paramLengths, const int* isBinary); 

      /**
       * Begin a transaction on this connection. Autocommit is ENABLED for all
       * executed commands UNLESS beginTransaction has been called--that is, beginTransaction
       * disabled autocommit until either rollbackTransaction or commitTransaction
       * is called.
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
       * @returns ERROR upon failure; otherwise the column Index >= 0
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
      string getStringValueAt(int rowIndex, int columnIndex);

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
       * commandResult. The value returned will be a string.
       * NOTE: Use of this method requires that the developer be aware of the
       * of the schema and result field type.
       */
      string getTimestampValueAt(int rowIndex, int columnIndex);

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
       * @returns ERROR on failure; otherwise OK
       */
      int shutdown();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /** Associate a connection object (and connectionSet name) with this connection Handler */
      void associateConnection(DbConnection* connection, string& connectionSetName);

      /** Remove the associated connection from this connection Handler and return it */
      DbConnection* dissociateConnection();

      /** Return a reference to the owning Connection Set name. This is required
          for lookup in the DataManager class to find the ConnectionSet in the Map */
      string& getConnectionSetName();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      DbConnectionHandle(const DbConnectionHandle& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DbConnectionHandle& operator= (const DbConnectionHandle& rhs);

      /** Associated DbConnection object */
      DbConnection* associatedConnection_;

      /** Owning Connection Set Name */
      string owningConnectionSetName_;

      /** Default error return string */
      string errorString_;

      /** Maintain and track the Thread Id for the Thread that reserved the connection. If a different
          application thread performs operations on the connection, the give a warning. Note, this 
          variable is an int. */
      ACE_thread_t ownerThread_;
};

#endif
