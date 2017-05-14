/******************************************************************************
*
* File name:   PgConnection.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sstream>
#include <iostream>

// NEED to do some hack crap to get the Postgres conversion header file to work!
// These guys need to document this stuff. We do this otherwise we have big
// problems of dealing with the 'long long' time value that comes out of the database
// -ALSO- We need to make sure that /var/lib/pgsql/data/postgresql.conf is setup
// to either use the OS time zone setting or we set it up manually in that file.
// Perhaps no longer needed with Postgres 9.1:
//#define HAVE_INT64_TIMESTAMP
//#define int64 long long
#include <pgtypes_timestamp.h>

#include <netinet/in.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "PgConnection.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


// For verifying the field/column types, Postgres uses internal OID values
// (which may change from version to version). These OID field type values are
// taken from the Postgresql.org source code (for version 8.0.3) in the file
// src/include/catalog/pg_types.h:
//
// NOTE: This can be determined programmatically by doing something like: (TODO)
// 
//
//       /* Read internal codes */
//     res = PQexec(pg_conn, "select oid, typname from pg_type where typname in ( "
//	         "'int2', 'int4', 'int8', 'serial', 'oid', "
//		 "'float4', 'float8', 'numeric', "
//		 "'char', 'bpchar', 'varchar', 'text', "
//		 "'time', 'date', 'timestamp', "
//	   	 "'bool' ) order by oid" );
// 
//     if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
// 	append_error ( "Cannot select data types" );
// 	    type = PG_TYPE_INT2;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "int4" ) == 0 )
// 	    type = PG_TYPE_INT4;
//	else if ( strcmp( PQgetvalue(res, row, 1), "int8" ) == 0 )
//	    type = PG_TYPE_INT8;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "serial" ) == 0 )
// 	    type = PG_TYPE_SERIAL;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "oid" ) == 0 )
// 	    type = PG_TYPE_OID;
//	else if ( strcmp( PQgetvalue(res, row, 1), "float4" ) == 0 )
//	    type = PG_TYPE_FLOAT4;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "float8" ) == 0 )
// 	    type = PG_TYPE_FLOAT8;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "numeric" ) == 0 )
// 	    type = PG_TYPE_BPCHAR;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "varchar" ) == 0 )
// 	    type = PG_TYPE_VARCHAR;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "text" ) == 0 )
// 	    type = PG_TYPE_TEXT;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "date" ) == 0 )
// 	    type = PG_TYPE_DATE;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "time" ) == 0 )
// 	    type = PG_TYPE_TIME;
// 	else if ( strcmp( PQgetvalue(res, row, 1), "timestamp" ) == 0 )
// 	    type = PG_TYPE_TIMESTAMP;
//	else if ( strcmp( PQgetvalue(res, row, 1), "bool" ) == 0 )
//	    type = PG_TYPE_BOOL;
// 	else 
// 	    type = PG_TYPE_UNKNOWN;
//
//
//
//
//
//
// Boolean:
#define BOOLOID                 16
// Variable length string, binary values escaped
#define BYTEAOID                17
// Single Character:
#define CHAROID                 18
// 63-Character type for system identifiers
#define NAMEOID                 19
// 18 digit integer, 8-byte storage
#define INT8OID                 20
// -32000 to 32000, 2-byte storage
#define INT2OID                 21
// Array of int2 integers used in system tables
#define INT2VECTOROID           22
// -2 billion to 2 billion integer, 4-byte storage
#define INT4OID                 23
// Registered procedure
#define REGPROCOID              24
// Variable length string, no limit specified
#define TEXTOID                 25
// OID type, maximum 4 billion
#define OIDOID                  26
// Physical location of tuple (block, offset)
#define TIDOID                  27
// Transaction Id
#define XIDOID                  28
// Command identifier, sequence in transaction id
#define CIDOID                  29
// Array of oids used in system tables
#define OIDVECTOROID            30
// single-precision floating point number, 4-byte storage
#define FLOAT4OID               700
// double-precision floating point number, 8-byte storage
#define FLOAT8OID               701
// absolute, limited-range date and time (Unix system time)
#define ABSTIMEOID              702
// relative, limited-range time interval (Unix delta time)
#define RELTIMEOID              703
// (abstime,abstime), time interval
#define TINTERVALOID            704
// XX:XX:XX:XX:XX:XX, MAC address
#define MACADDROID              829
// IP address/netmask, host address, netmask optional
#define INETOID                 869
// network IP address/netmask, network address
#define CIDROID                 650
// Integer 4byte array
#define INT4ARRAYOID            1007
// access control list
#define ACLITEMOID              1033
// char(length), blank-padded string, fixed storage length
#define BPCHAROID               1042
// varchar(length), non-blank-padded string, variable storage length
#define VARCHAROID              1043
// ANSI SQL date
#define DATEOID                 1082
// hh:mm:ss, ANSI SQL time
#define TIMEOID                 1083
// date and time
#define TIMESTAMPOID            1114
// date and time with time zone
#define TIMESTAMPTZOID          1184
// @ <number> <units>, time interval
#define INTERVALOID             1186
// hh:mm:ss, ANSI SQL time
#define TIMETZOID               1266
// fixed-length bit string
#define BITOID                  1560
// variable-length bit string
#define VARBITOID               1562
// numeric(precision, decimal), arbitrary precision number
#define NUMERICOID              1700


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
PgConnection::PgConnection()
             : pgConn_(NULL),
               pgResult_(NULL),
               errorString_("DEFAULT_ERROR_STRING")
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
PgConnection::~PgConnection()
{
   // Close the connection to the server
   PQfinish(pgConn_);
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method will be called when the pool is created with these objects
// Design: OPM object creation MUST be successful. If the connection cannot be
//         made successfully, return the connection object anyway and we will
//         try again later.
//-----------------------------------------------------------------------------
OPMBase* PgConnection::initialize(int initializer)
{
   // Create a new instance of the PgConnection
   PgConnection* connection = new PgConnection();

   // Parse out the initialization parameters
   PgInitParams* pgInitParams = (PgInitParams*)initializer;

   // Attempt to establish the database connection
   connection->establishConnection(pgInitParams->userid, pgInitParams->password, 
      pgInitParams->dsn, pgInitParams->connectionString);
  
   // Return the connection object regardless of pass/fail of connection establishment
   return connection;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Pure Virtual method will be called before releasing the object
//              back into the OPM pool
// Design:
//-----------------------------------------------------------------------------
void PgConnection::clean()
{
   // Need to make sure that we release every resultset!! Otherwise memory leak.
   closeCommandResult();
 
   // Check the health of the database connection. If the health check fails,
   // then before we release the connection back into the pool, we need to fix it!!
   if (checkConnectionHealth() == ERROR)
   {
      // Check to ensure that the connection is now healthy
      //if (reEstablishConnection() == ERROR)
      //{
      //   // Sleep for 2 seconds and then re-Attempt once more
      //   TRACELOG(ERRORLOG, DATAMGRLOG, "Sleeping before attempting connection reEstablishment once more",0,0,0,0,0,0);
      //   sleep(15);
      //   reEstablishConnection();
      //}//end if

      // Rather than MANUALLY try to re-establish the connection, we can get the driver
      // to try it
      TRACELOG(ERRORLOG, DATAMGRLOG, "Attempting reset on a connection",0,0,0,0,0,0);
      PQreset(pgConn_);
   }//end if
}//end clean


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Check the health of the database connection
// Design:
//-----------------------------------------------------------------------------
int PgConnection::checkConnectionHealth()
{
   if (getConnectionStatus() != CONNECTION_OK)
   {
      ostringstream ostr;
      ostr << "Connection health status check failed with error: "
           << PQerrorMessage(pgConn_) << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      return ERROR;
   }//end if
   return OK;
}//end


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL command to the database using this connection.
// Design:
//-----------------------------------------------------------------------------
int PgConnection::executeCommand(string& command)
{
   // Before we execute another command, check to make sure that the previous
   // result set was deallocated (otherwise we will create a memory leak)
   closeCommandResult();

   // Execute the command. Here we could use 'PQexec', but it won't give us the
   // ability to request results in binary format. If we don't specify '1' here,
   // then all of the results will be translated from their native formats inside
   // the database into textual strings in the resultset (and we would have to
   // convert back if we needed them in another format).
   pgResult_ = PQexecParams(pgConn_, command.c_str(), 0, NULL, NULL, NULL, NULL, 1);
   //pgResult_ = PQexec(pgConn_, command.c_str());

   // Check the result
   if ( (pgResult_ != NULL) && (PQresultStatus(pgResult_) != PGRES_COMMAND_OK) &&
        (PQresultStatus(pgResult_) != PGRES_TUPLES_OK) )
   {
      ostringstream ostr;
      ostr << "Command execution failed for '" << command
           << "' with error: " << PQerrorMessage(pgConn_) << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      closeCommandResult();
      return ERROR;
   }//end if
   return OK;
}//end executeCommand


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL command to the database using this connection.
// Design:
//-----------------------------------------------------------------------------
int PgConnection::executeCommand(const char* command)
{
   // Before we execute another command, check to make sure that the previous
   // result set was deallocated (otherwise we will create a memory leak)
   closeCommandResult();

   // Execute the command. Here we could use 'PQexec', but it won't give us the
   // ability to request results in binary format. If we don't specify '1' here,
   // then all of the results will be translated from their native formats inside
   // the database into textual strings in the resultset (and we would have to
   // convert back if we needed them in another format).
   pgResult_ = PQexecParams(pgConn_, command, 0, NULL, NULL, NULL, NULL, 1);
   //pgResult_ = PQexec(pgConn_, command);

   // Check the result
   if ( (pgResult_ != NULL) && (PQresultStatus(pgResult_) != PGRES_COMMAND_OK) &&
        (PQresultStatus(pgResult_) != PGRES_TUPLES_OK) )
   {
      ostringstream ostr;
      ostr << "Command execution failed for '" << command
           << "' with error: " << PQerrorMessage(pgConn_) << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      closeCommandResult();
      return ERROR;
   }//end if
   return OK;
}//end executeCommand


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL command to the database using this connection.
// Design:
//-----------------------------------------------------------------------------
int PgConnection::executeCommandWithParams(const char* command, int numberParams, 
    const char* const* paramValues, const int* paramLengths, const int* isBinary)
{
   // Before we execute another command, check to make sure that the previous
   // result set was deallocated (otherwise we will create a memory leak)
   closeCommandResult();

   // Execute the command. Here we could use 'PQexec', but it won't give us the
   // ability to request results in binary format. If we don't specify '1' here,
   // then all of the results will be translated from their native formats inside
   // the database into textual strings in the resultset (and we would have to
   // convert back if we needed them in another format).
   //
   // Here we let the backend deduce param type 
   pgResult_ = PQexecParams(pgConn_, command, numberParams, NULL, paramValues, paramLengths, isBinary, 1);

   // Check the result
   if ( (pgResult_ != NULL) && (PQresultStatus(pgResult_) != PGRES_COMMAND_OK) && 
        (PQresultStatus(pgResult_) != PGRES_TUPLES_OK) )
   {
      ostringstream ostr;
      ostr << "Command execution failed for '" << command
           << "' with error: " << PQerrorMessage(pgConn_) << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      closeCommandResult();
      return ERROR;
   }//end if
   return OK; 
}//end executeCommandWithParams


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Begin a transaction on this connection.
// Design:
//-----------------------------------------------------------------------------
int PgConnection::beginTransaction()
{
   return executeCommand("BEGIN;");
}//end beginTransaction
                                                                                                                               
                                                                                                                               
//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Rollback a transaction on this connection
// Design:
//-----------------------------------------------------------------------------
int PgConnection::rollbackTransaction()
{
   return executeCommand("ROLLBACK;");
}//end rollbackTransaction


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Commit the outstanding transaction on this connection
// Design:
//-----------------------------------------------------------------------------
int PgConnection::commitTransaction()
{
   return executeCommand("COMMIT;");
}//end commitTransaction


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup for a prepared statement command on this connection
// Design:
//-----------------------------------------------------------------------------
int PgConnection::initializePreparedCommand(const char* statementName, const char* sqlCommand,
            int numberParams, const DataManagerFieldType* paramTypeArray)
{
   // Before we execute another command, check to make sure that the previous
   // result set was deallocated (otherwise we will create a memory leak)
   closeCommandResult();

   // Convert the field types to Postgres Field types
   Oid paramTypes[numberParams];
   for (int paramNumber = 0; paramNumber < numberParams; paramNumber++)
   {
      switch (*paramTypeArray)
      {
         case (BOOLEAN_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = BOOLOID;
            break;
         }//end case
         case (CHAR_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = CHAROID;
            break;
         }//end case
         case (INT8_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = INT8OID;
            break;
         }//end case
         case (INT4_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = INT4OID;
            break;
         }//end case
         case (INT2_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = INT2OID;
            break;
         }//end case
         case (FLOAT4_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = FLOAT4OID;
            break;
         }//end case
         case (FLOAT8_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = FLOAT8OID;
            break;
         }//end case
         case (VARCHAR_FIELD_TYPE) :
         {
            paramTypes[paramNumber] = VARCHAROID;
            break;
         }//end case
         case (TIMESTAMP_FIELD_TYPE) :
         {
            // With time zone
            paramTypes[paramNumber] = TIMESTAMPTZOID;
            break;
         }//end case
         default : 
         {
            TRACELOG(ERRORLOG, DATAMGRLOG, "Unmapped field type: %d", *paramTypeArray,0,0,0,0,0);
            break;
         }//end default
      }//end switch
      paramTypeArray++;
   }//end for

   // Prepare the statement
   pgResult_ = PQprepare(pgConn_, statementName, sqlCommand, numberParams, paramTypes );

   if ( (pgResult_ != NULL) && (PQresultStatus(pgResult_) != PGRES_COMMAND_OK) &&
        (PQresultStatus(pgResult_) != PGRES_TUPLES_OK) )
   {
      ostringstream ostr;
      ostr << "Prepared statement command preparation failed for statement '" << statementName
           << "' with error: " << PQerrorMessage(pgConn_) << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      closeCommandResult();
      return ERROR;
   }//end if
   return OK;
}//end initializePreparedCommand


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Execute an SQL prepared statement command to the database using this connection
// Design:
//-----------------------------------------------------------------------------
int PgConnection::executePreparedCommand(const char* statementName, int numberParams,
            const char* const* paramValues, const int* paramLengths, const int* isBinary)
{
   // Before we execute another command, check to make sure that the previous
   // result set was deallocated (otherwise we will create a memory leak)
   closeCommandResult();

   // Execute the prepared statement - Get results in binary format
   pgResult_ = PQexecPrepared(pgConn_, statementName, numberParams, paramValues, paramLengths, isBinary, 1);

   // Check the result
   if ( (pgResult_ != NULL) && (PQresultStatus(pgResult_) != PGRES_COMMAND_OK) &&
        (PQresultStatus(pgResult_) != PGRES_TUPLES_OK) )
   {
      ostringstream ostr;
      ostr << "Prepared statement command execution failed for statement '" << statementName
           << "' with error: " << PQerrorMessage(pgConn_) << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      closeCommandResult();
      return ERROR;
   }//end if
   return OK;
}//end


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: If an outstanding/open command result exists for this connection, then close it
// Design:
//-----------------------------------------------------------------------------
int PgConnection::closeCommandResult()
{
   if (pgResult_ != NULL)
   {
      PQclear(pgResult_);
      pgResult_ = NULL;
   }//end if
   return OK;
}//end closeResultSet


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the number of columns in the current commandResult
// Design:
//-----------------------------------------------------------------------------
int PgConnection::getColumnCount()
{
   if (pgResult_ != NULL)
   {
      return PQnfields(pgResult_);
   }//end if
   return 0;
}//end getColumnCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the specified column header name in the current commandResult
// Design:
//-----------------------------------------------------------------------------
string PgConnection::getColumnName(int columnIndex)
{
   if (pgResult_ != NULL)
   {
      return PQfname(pgResult_, columnIndex);
   }//end if
   return errorString_; 
}//end getColumnName


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the column index based on the specified column name
// Design:
//-----------------------------------------------------------------------------
int PgConnection::getColumnIndex(const char* columnName)
{
   if (pgResult_ != NULL)
   {
      return PQfnumber(pgResult_, columnName);
   }//end if
   return ERROR;
}//end getColumnIndex


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the number of rows in the current commandResult
// Design:
//-----------------------------------------------------------------------------
int PgConnection::getRowCount()
{
   if (pgResult_ != NULL)
   {
      return PQntuples(pgResult_);
   }//end if
   return 0;
}//end getRowCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the 
//              current commandResult. The value is returned as a string.
// Design:
//-----------------------------------------------------------------------------
const char* PgConnection::getStringValueAt(int rowIndex, int columnIndex)
{
   char* tmpBuffer = NULL;

   if (pgResult_ != NULL)
   {
      // Check the format type of the column so we know that we are returning the appropriate type
      Oid columnOid = PQftype(pgResult_, columnIndex);
      if ( (columnOid == BYTEAOID) || (columnOid == CHAROID) || (columnOid == TEXTOID) ||
           (columnOid == BPCHAROID) || (columnOid == VARCHAROID) )
      {
         tmpBuffer = PQgetvalue(pgResult_, rowIndex, columnIndex);
      }//end if
      else
      {
         TRACELOG(ERRORLOG, DATAMGRLOG, "Invalid request for string value at %d,%d when column type is %d",
            rowIndex, columnIndex, columnOid,0,0,0);
         return errorString_;
      }//end else
   }//end if
   else
   {
      return errorString_;
   }//end else 
   return tmpBuffer;  
}//end getStringValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a short
// Design:
//-----------------------------------------------------------------------------
short PgConnection::getShortValueAt(int rowIndex, int columnIndex)
{
   char* tmpBuffer;
   if (pgResult_ != NULL)
   {
      // Check the format type of the column so we know that we are returning the appropriate type
      Oid columnOid = PQftype(pgResult_, columnIndex);
      if ( columnOid == INT2OID )
      {
         tmpBuffer = PQgetvalue(pgResult_, rowIndex, columnIndex);

         // convert from network byte order to local byte order and return the short value
         // NOTE: Do we need to be working with 'unsigned' values here?
         return (ntohs(*((short *) tmpBuffer)));
      }//end if
      else
      {
         TRACELOG(ERRORLOG, DATAMGRLOG, "Invalid request for short value at %d,%d when column type is %d",
            rowIndex, columnIndex, columnOid,0,0,0);
         return 0;
      }//end else
   }//end if
   else
   {
      return 0;
   }//end else
}//end getShortValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as an integer. 
// Design:
//-----------------------------------------------------------------------------
int PgConnection::getIntValueAt(int rowIndex, int columnIndex)
{
   char* tmpBuffer;
   if (pgResult_ != NULL)
   {
      // Check the format type of the column so we know that we are returning the appropriate type
      Oid columnOid = PQftype(pgResult_, columnIndex);
      if ( columnOid == INT4OID )
      {
         tmpBuffer = PQgetvalue(pgResult_, rowIndex, columnIndex);
         // convert from network byte order to local byte order and return the integer value
         // NOTE: Do we need to be working with 'unsigned' values here?
         return (ntohl(*((int *) tmpBuffer)));
      }//end if
      else
      {
         TRACELOG(ERRORLOG, DATAMGRLOG, "Invalid request for integer value at %d,%d when column type is %d", 
            rowIndex, columnIndex, columnOid,0,0,0);
         return 0;
      }//end else
   }//end if
   else
   {
      return 0;
   }//end else
}//end getIntValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a boolean.
// Design:
//-----------------------------------------------------------------------------
bool PgConnection::getBooleanValueAt(int rowIndex, int columnIndex)
{
   char* tmpBuffer;
   if (pgResult_ != NULL)
   {
      // Check the format type of the column so we know that we are returning the appropriate type
      Oid columnOid = PQftype(pgResult_, columnIndex);
      if ( columnOid == BOOLOID )
      {
         tmpBuffer = PQgetvalue(pgResult_, rowIndex, columnIndex);
         // convert from network byte order to local byte order and return the boolean value
         return (*((bool *) tmpBuffer));
      }//end if
      else
      {
         TRACELOG(ERRORLOG, DATAMGRLOG, "Invalid request for boolean value at %d,%d when column type is %d",
            rowIndex, columnIndex, columnOid,0,0,0);
         return false;
      }//end else
   }//end if
   else
   {
      return false;
   }//end else
}//end getBooleanValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the value at the specified column/row location in the
//              current commandResult. The value is returned as a string.
// Design:
//-----------------------------------------------------------------------------
const char* PgConnection::getTimestampValueAt(int rowIndex, int columnIndex)
{
   char* tmpBuffer;
   if (pgResult_ != NULL)
   {
      // Check the format type of the column so we know that we are returning the appropriate type
      Oid columnOid = PQftype(pgResult_, columnIndex);
      if ( columnOid == TIMESTAMPTZOID )
      {
         tmpBuffer = PQgetvalue(pgResult_, rowIndex, columnIndex);
         // convert from network byte order to local byte order and return the long long value,
         // then convert to string
         timestamp tstamp = (ntohll(*((long long *) tmpBuffer)));
         return PGTYPEStimestamp_to_asc(tstamp);
      }//end if
      else
      {
         TRACELOG(ERRORLOG, DATAMGRLOG, "Invalid request for Timestamp (with time zone) at %d,%d when column type is %d",
            rowIndex, columnIndex, columnOid,0,0,0);
         return errorString_;
      }//end else
   }//end if
   else
   {
      return errorString_;
   }//end else
}//end getTimestampValueAt


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns true if the field Value is NULL
// Design:
//-----------------------------------------------------------------------------
bool PgConnection::isValueNULL(int rowIndex, int columnIndex)
{
   int result = 0;
   if (pgResult_ != NULL)
   {
      result = PQgetisnull(pgResult_, rowIndex, columnIndex);
   }//end if
   
   if (result == 1)
   {
      return true;
   }//end if
   return false;
}//end isValueNULL
 

//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the actual length of a field value in bytes
// Design:
//-----------------------------------------------------------------------------
int PgConnection::getValueLength(int rowIndex, int columnIndex)
{
   if (pgResult_ != NULL)
   {
      return PQgetlength(pgResult_, rowIndex, columnIndex);
   }//end if
   return 0;
}//end getValueLength


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Shutdown down the database connection
// Design:
//-----------------------------------------------------------------------------
void PgConnection::shutdown()
{
   // Close any open result set
   closeCommandResult();
   // Close the connection to the server
   PQfinish(pgConn_);
}//end shutdown


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string PgConnection::toString()
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
// Description: Perform the initial connection establishment with the database
// Design:
//-----------------------------------------------------------------------------
int PgConnection::establishConnection(string userid, string password, string dsn, string connectionString)
{
   // store the connection parameters locally
   userid_ = userid;
   password_ = password;
   dsn_ = dsn;
   connectionString_ = connectionString;

   // perform the connection 
   return reEstablishConnection();
}//end establishConnection


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: ReEstablish the database connection
// Design:
//-----------------------------------------------------------------------------
int PgConnection::reEstablishConnection()
{
   // Assemble the connection parameters into a single string
   string postgresConnectString = connectionString_ +
      " user=" + userid_ + " password=" + password_ + " dbname=" + dsn_;

   ostringstream ostr;
   ostr << "Preparing to call postgresql database PQconnectdb with connect string: ("
        << postgresConnectString << ")" << ends;
   STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());

   // Perform the connection to the database
   pgConn_ = PQconnectdb(postgresConnectString.c_str());

   // This should never happen. Always supposed to return non-NULL
   if (pgConn_ == NULL)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Postgresql Connection object is NULL",0,0,0,0,0,0);
      return ERROR;
   }//end if
   // Check the health status of the Connection; should be only CONNECTION_OK or CONNECTION_BAD
   else if (getConnectionStatus() != CONNECTION_OK)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Postgresql Bad Connection detected",0,0,0,0,0,0);
      return ERROR;
   }//end else if
   else
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql Connection Established",0,0,0,0,0,0);
   }//end else

   return OK;
}//end reEstablishConnection


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Interpret the PQStatus result
// Design:
//-----------------------------------------------------------------------------
int PgConnection::getConnectionStatus()
{
   int status = PQstatus(pgConn_);

   // Generate a log to indicate which of the supported connection statuses are set
   switch (status)
   {
      /* These are for blocking mode connections: */
      case CONNECTION_OK:
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION OK",0,0,0,0,0,0);
         break;
      case CONNECTION_BAD:
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION BAD",0,0,0,0,0,0);
         break;
      /* These are for Non-blocking mode connections only: */
      case CONNECTION_STARTED:
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION IN PROGRESS",0,0,0,0,0,0);
         break;
      case CONNECTION_MADE:
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION MADE, WAITING TO SEND",0,0,0,0,0,0);
         break;
      case CONNECTION_AWAITING_RESPONSE:
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION AWAITING RESPONSE",0,0,0,0,0,0);
         break;
      case CONNECTION_AUTH_OK:
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION RECEIVED AUTHENTICATION, BACKEND STARTING",0,0,0,0,0,0);
         break;
      //case CONNECTION_SSL_STARTUP:
      //   TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION SSL STARTUP",0,0,0,0,0,0);
      //   break;
      case CONNECTION_SETENV:
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Postgresql connection status: CONNECTION NEGOTIATING ENVIRONMENT",0,0,0,0,0,0);
         break;
      default:
         TRACELOG(WARNINGLOG, DATAMGRLOG, "Postgresql connection status: UNKNOWN",0,0,0,0,0,0);
         break; 
   }//end switch
   return status;
}//end getConnectionStatus


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

