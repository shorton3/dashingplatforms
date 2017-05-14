/******************************************************************************
*
* File name:   DbConnectionHandle.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DbConnectionHandle.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DbConnectionHandle::DbConnectionHandle() : associatedConnection_(NULL),
                                           errorString_("DEFAULT_ERROR_STRING"),
                                           ownerThread_(0)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DbConnectionHandle::~DbConnectionHandle()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method will be called when the pool is created with these objects
// Design: OPM object creation MUST be successful.
//-----------------------------------------------------------------------------
OPMBase* DbConnectionHandle::initialize(int initializer)
{
   // Handle unused variable warning
   initializer = 0;

   // Create a new instance of the DbConnectionHandle
   DbConnectionHandle* connection = new DbConnectionHandle();

   // Return the connection object
   return connection;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Pure Virtual method will be called before releasing the object
//              back into the OPM pool
// Design:      NOTE: do NOT clear the userDefinedIdentifierString_
//-----------------------------------------------------------------------------
void DbConnectionHandle::clean()
{
   associatedConnection_ = NULL;
   ownerThread_ = 0;
}//end clean


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Allows the Application Developer to 'tag' a particular connection
// Design:
//-----------------------------------------------------------------------------
void DbConnectionHandle::addUserDefinedIdentifier(string& identifier)
{
   if (associatedConnection_ != NULL)
   {
      associatedConnection_->addUserDefinedIdentifier(identifier);
   }//end if
}//end addUserDefinedIdentifier


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the user-defined identifier data
// Design:
//-----------------------------------------------------------------------------
string& DbConnectionHandle::getUserDefinedIdentifier()
{
   // We should NOT have to check this
   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getUserDefinedIdentifier();
   }//end if
   else
   {
      return errorString_;
   }//end else
}//end getUserDefinedIdentifier


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Clears the user-defined data and resets the internal string to empty
// Design:
//-----------------------------------------------------------------------------
void DbConnectionHandle::clearUserDefinedIdentifier()
{
   if (associatedConnection_ != NULL)
   {
      associatedConnection_->clearUserDefinedIdentifier();
   }//end if
}//end clearUserDefinedIdentifier


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Check the health of the database connection
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::checkConnectionHealth()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "checkConnectionHealth() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->checkConnectionHealth();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL command to the database using this connection.
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::executeCommand(string& command)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "executeCommand() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->executeCommand(command);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end executeCommand


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL command to the database using this connection.
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::executeCommand(const char* command)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "executeCommand() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->executeCommand(command);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end executeCommand


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL command to the database using this connection.
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::executeCommandWithParams(const char* command, int numberParams,
            const char* const* paramValues, const int* paramLengths, const int* isBinary)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "executeCommandWithParams() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->executeCommandWithParams(command, numberParams,
         paramValues, paramLengths, isBinary);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end executeCommandWithParams


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Begin a transaction on this connection.
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::beginTransaction()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "beginTransaction() called by a different thread",0,0,0,0,0,0);
   }//end if

   TRACELOG(DEVELOPERLOG, DATAMGRLOG, "Beginning Database Transaction",0,0,0,0,0,0);

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->beginTransaction();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end beginTransaction


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Rollback a transaction on this connection
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::rollbackTransaction()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "rollbackTransaction() called by a different thread",0,0,0,0,0,0);
   }//end if

   TRACELOG(DEVELOPERLOG, DATAMGRLOG, "Rolling Back Database Transaction",0,0,0,0,0,0);

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->rollbackTransaction();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end rollbackTransaction


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Commit the outstanding transaction on this connection
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::commitTransaction()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "commitTransaction() called by a different thread",0,0,0,0,0,0);
   }//end if

   TRACELOG(DEVELOPERLOG, DATAMGRLOG, "Committing Database Transaction",0,0,0,0,0,0);

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->commitTransaction();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end commitTransaction


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup for a prepared statement command on this connection
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::initializePreparedCommand(const char* statementName,
    const char* sqlCommand, int numberParams, const DataManagerFieldType* paramTypeArray)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "initializePreparedCommand() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->initializePreparedCommand(statementName, sqlCommand, numberParams, paramTypeArray);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end initializePreparedCommand


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL prepared statement command to the database using this connection
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::executePreparedCommand(const char* statementName, int numberParams,
    const char* const* paramValues, const int* paramLengths, const int* isBinary)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "executePreparedCommand() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->executePreparedCommand(statementName, numberParams, paramValues, paramLengths, isBinary);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end executePreparedCommand


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: If an outstanding/open command result exists for this connection, then close it
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::closeCommandResult()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "closeCommandResult() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->closeCommandResult();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end closeCommandResult


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the number of columns in the current commandResult
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::getColumnCount()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getColumnCount() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getColumnCount();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end getColumnCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the specified column header name in the current commandResult
// Design:
//-----------------------------------------------------------------------------
string DbConnectionHandle::getColumnName(int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getColumnName() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getColumnName(columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return errorString_;
   }//end else
}//end getColumnName


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the column index based on the specified column name
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::getColumnIndex(const char* columnName)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getColumnIndex() called by a different thread",0,0,0,0,0,0);
   }//end if
                                                                                                                                  
   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getColumnIndex(columnName);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end getColumnIndex


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the number of rows in the current commandResult
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::getRowCount()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getRowCount() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getRowCount();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end getRowCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a string.
// Design:
//-----------------------------------------------------------------------------
string DbConnectionHandle::getStringValueAt(int rowIndex, int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getStringValueAt() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getStringValueAt(rowIndex, columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return errorString_;
   }//end else
}//end getStringValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a 2-byte short type
// Design:
//-----------------------------------------------------------------------------
short DbConnectionHandle::getShortValueAt(int rowIndex, int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getShortValueAt() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getShortValueAt(rowIndex, columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else 
}//end getShortValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a 4-byte integer type
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::getIntValueAt(int rowIndex, int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getIntValueAt() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getIntValueAt(rowIndex, columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end getIntValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a boolean type
// Design:
//-----------------------------------------------------------------------------
bool DbConnectionHandle::getBooleanValueAt(int rowIndex, int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getBooleanValueAt() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getBooleanValueAt(rowIndex, columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return false;
   }//end else
}//end getBooleanValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a string.
// Design:
//-----------------------------------------------------------------------------
string DbConnectionHandle::getTimestampValueAt(int rowIndex, int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getTimestampValueAt() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getTimestampValueAt(rowIndex, columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return 0;
   }//end else
}//end getTimestampValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns true if the field Value is NULL
// Design:
//-----------------------------------------------------------------------------
bool DbConnectionHandle::isValueNULL(int rowIndex, int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "isValueNULL() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->isValueNULL(rowIndex, columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end isValueNULL


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the actual length of a field value in bytes
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::getValueLength(int rowIndex, int columnIndex)
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "getValueLength() called by a different thread",0,0,0,0,0,0);
   }//end if

   if (associatedConnection_ != NULL)
   {
      return associatedConnection_->getValueLength(rowIndex, columnIndex);
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR;
   }//end else
}//end getValueLength


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Close the socket connection to the database server
// Design:
//-----------------------------------------------------------------------------
int DbConnectionHandle::shutdown()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "shutdown() called by a different thread",0,0,0,0,0,0);
   }//end if
                                                                                                                            
   if (associatedConnection_ != NULL)
   {
      associatedConnection_->shutdown();
      return OK;
   }//end if
   else
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "No associated connection for handle",0,0,0,0,0,0);
      return ERROR; 
   }//end else
}//end shutdown 


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string DbConnectionHandle::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Associate a connection object with this connection Handler
// Design:
//-----------------------------------------------------------------------------
void DbConnectionHandle::associateConnection(DbConnection* connection, string& connectionSetName)
{
   associatedConnection_ = connection;
   // This is a string copy that 'can' occur in the application critical path 'if'
   // DB connection reserve / release is occuring in the critical path. As an optimization,
   // couldn't we just pass in a pointer to the connectionSetName_ member variable inside ConnectionSet?
   owningConnectionSetName_ = connectionSetName;
   // Here we set the Owning Thread Id to the thread that reserved the connection. If
   // another thread performs operations on this connection, then we will give a WARNING
   // as it may indicate the application is attempting to maintain a pointer to a database
   // connection that has already been released back into the pool
   ownerThread_ = ACE_Thread::self();
}//end associateConnection


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Remove the associated connection from this connection Handler and return it
// Design:
//-----------------------------------------------------------------------------
DbConnection* DbConnectionHandle::dissociateConnection()
{
   // Here we check to see if the calling thread is the same as the thread that reserved
   // the connection. If not, this may indicate that the application is attempting to maintain
   // a pointer to a connection that has already been released back into the pool.
   if (ownerThread_ != ACE_Thread::self())
   {
      TRACELOG(WARNINGLOG, DATAMGRLOG, "dissociateConnection() called by a different thread",0,0,0,0,0,0);
   }//end if

   // Make a copy of the member variable pointer and Null it out (return the copy)
   DbConnection* tempConnection = associatedConnection_;
   associatedConnection_ = NULL;
   return tempConnection;
}//end dissociateConnection


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the owning connection set name
// Design:
//-----------------------------------------------------------------------------
string& DbConnectionHandle::getConnectionSetName()
{
   return owningConnectionSetName_;
}//end getConnectionSetName


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

