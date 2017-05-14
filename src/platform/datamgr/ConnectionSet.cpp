/******************************************************************************
*
* File name:   ConnectionSet.cpp
* Subsystem:   Platform Services
* Description: Class handles activation and deactivation of the database
*              connections (including connection establishment and error handling).
*              Also interacts with OPM for connection pooling.
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

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ConnectionSet.h"
#include "PgConnection.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

#include "platform/opm/OPM.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Pool Id for the pool of connection Handle objects (used by all ConnectionSets)
int ConnectionSet::handleObjectPoolId_ = 0;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ConnectionSet::ConnectionSet(string connectionSetName) 
              : connectionSetName_(connectionSetName),
                connectionType_(UNKNOWN_CONNECTIONTYPE),
                numberConnections_(0),
                connectionObjectPoolId_(0)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ConnectionSet::~ConnectionSet()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the Userid associated with the connection
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::setUserid(string userid)
{
   userid_ = userid;
}//end setUserid


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the Password associated with the connection
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::setPassword(string password)
{
   password_ = password;
}//end setPassword


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the DriverName associated with the connection
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::setDriverName(string driverName)
{
   driverName_ = driverName;
}//end setDriverName


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the ConnectionType associated with the connection
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::setConnectionType(ConnectionSetType connectionType)
{
   connectionType_ = connectionType;
}//end setConnectionType   


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the Connection String associated with the connection
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::setConnectionString(string connectionString)
{
   connectionString_ = connectionString;
}//end setConnectionString


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the number of connections
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::setNumberConnections(int numberConnections)
{
   numberConnections_ = numberConnections;
}//end setNumberConnections


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the DSN
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::setDSN(string dsn)
{
   dsn_ = dsn;
}//end setDSN


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform activation for the connection set
// Design:
//-----------------------------------------------------------------------------
int ConnectionSet::performActivation()
{
   // Validate the number of connections
   if (numberConnections_ < 1)
   {
      TRACELOG(ERRORLOG, DATAMGRLOG, "Invalid number of connections specified: %d", numberConnections_,0,0,0,0,0);
      return ERROR;
   }//end if

   // Determine which connection object to use based on the driver Name specified
   // in the configuration file. Currently, we only support PgConnection (for the Postgresql database)
   if (driverName_ == "libpq")
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "Creating connection objects for Postgresql Database",0,0,0,0,0,0);

      // Create a structure for storing the initialization parameters in order
      // to pass into the OPM createPool method (as a pointer to this struct)
      PgInitParams pgInitParams;
      pgInitParams.userid = userid_;
      pgInitParams.password = password_;
      pgInitParams.connectionString = connectionString_;
      pgInitParams.dsn = dsn_;

      // Create a Thread Safe OPM pool for the number of connections specified
      // Use the ConnectionSet Name as the PoolName. Pass in a pointer to a PgInitParams struct.
      // NOTE: OPM be initialized prior to this step!!!
      connectionObjectPoolId_ = OPM::createPool(connectionSetName_.c_str(),
         (long)&pgInitParams, (OPM_INIT_PTR)&PgConnection::initialize, 0.8, 1,
         numberConnections_, true, OPM_NO_GROWTH);
      if (connectionObjectPoolId_ == ERROR)
      {
         ostringstream ostr;
         ostr << "Error creating connection pool for (" << connectionSetName_ << ")" << ends;
         STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
         return ERROR;
      }//end if
      else
      {
         ostringstream ostr;
         ostr << "Created connection pool for (" << connectionSetName_ << ")" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
      }//end else
   }//end if
   else
   {
      ostringstream ostr;
      ostr << "Unrecognized driver name specified in DataManager configuration file: (" << driverName_ << ")" << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      return ERROR;
   }//end else

   // The first time that 'performActivation' is called on some ConnectionSet, then we lazily
   // create the pool of connection handle objects for ALL ConnectionSets. Note that
   // handleObjectPoolId_ is static
   if (handleObjectPoolId_ == 0)
   {
      TRACELOG(DEBUGLOG, DATAMGRLOG, "Creating connection handle objects for All ConnectionSets",0,0,0,0,0,0);
      // Let the capacity of this pool GROW as needed. Configure the pool to grow 1 at a time, and
      // set the initial size of the pool to 10 handlers (Note that this is 10 handlers PER PROCESS)
      handleObjectPoolId_ = OPM::createPool("DbConnectionHandle", 0, (OPM_INIT_PTR)&DbConnectionHandle::initialize,
         0.8, 1 , 10 , true, OPM_GROWTH_ALLOWED);
      if (handleObjectPoolId_ == ERROR)
      {
         TRACELOG(ERRORLOG, DATAMGRLOG, "Error creating connection handle pool",0,0,0,0,0,0);
         return ERROR;
      }//end if
      else
      {
         ostringstream ostr;
         ostr << "Created connection pool for connection handle objects" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
      }//end else
   }//end if

   // Now, we are ready to RESERVE and RELEASE connection objects for performing database operations
   return OK;
}//end performActivation


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform deactivation for the connection set
// Design:
//-----------------------------------------------------------------------------
int ConnectionSet::performDeactivation()
{
   // Currently there is no OPM support for deallocating the pool of objects
   // But we can loop through all of the 'Available' connection objects and
   // perform a disconnect/shutdown. HOWEVER, this does assume that the application
   // has released all of the connection objects back into the pool already
   // (so that we can re-reserve them here)
   DbConnectionHandle* connectionHandle = reserveConnectionNonBlocking();
   while (connectionHandle != NULL)
   {
      // Tear down the connection
      connectionHandle->shutdown();

      // Retrieve the next connection handle
      connectionHandle = reserveConnectionNonBlocking();
   }//end while

   // NOTE!!! Here, since we don't try to keep track of the connectionHandle objects
   // so that we can later dissociate and return -only- the handle objects back into
   // the pool (we don't want to put the DbConnections back into the pool since the
   // health check will attempt a reconnect), then we WILL BE LEAKING BOTH HANDLE
   // AND CONNECTION OBJECTS in the OPM.

   return OK;
}//end performDeactivation


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to retrieve a pointer to a DbConnection from the Connection Set pool
// Design:
//-----------------------------------------------------------------------------
DbConnectionHandle* ConnectionSet::reserveConnection()
{
   // Reserve a connection from the OPM based on the connectionObjectPoolId
   DbConnection* connection = (DbConnection*)OPM_RESERVE(connectionObjectPoolId_);

   // Reserve a connection handle from the OPM
   DbConnectionHandle* connectionHandle = (DbConnectionHandle*)OPM_RESERVE(handleObjectPoolId_);

   // Store the connection inside the connection handle. ConnectionSet is a 'friend'
   // of the DbConnectionHandle and so can call associateConnection
   connectionHandle->associateConnection(connection, connectionSetName_);

   // Return a pointer to the connection handle to the application developer
   return connectionHandle;
}//end reserveConnection


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve a pointer to a DbConnection from the pool in a non-blocking manner
// Design:
//-----------------------------------------------------------------------------
DbConnectionHandle* ConnectionSet::reserveConnectionNonBlocking()
{
   // Reserve a connection from the OPM
   DbConnection* connection = (DbConnection*)OPM_RESERVE_NONBLOCKED(connectionObjectPoolId_);

   // Reserve a connection handle from the OPM
   DbConnectionHandle* connectionHandle = (DbConnectionHandle*)OPM_RESERVE(handleObjectPoolId_);

   // Store the connection inside the connection handle. ConnectionSet is a 'friend'
   // of the DbConnectionHandle and so can call associateConnection
   connectionHandle->associateConnection(connection, connectionSetName_);

   // Return a pointer to the connection handle to the application developer
   return connectionHandle;
}//end reserveConnectionNonBlocking


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to release the DbConnection back into the Connection Set pool
// Design:
//-----------------------------------------------------------------------------
void ConnectionSet::releaseConnection(DbConnectionHandle* connectionHandle)
{
   // Retrieve the connect from the connection handle.  ConnectionSet is a 'friend'
   // of the DbConnectionHandle and so can call dissociateConnection
   DbConnection* connection = connectionHandle->dissociateConnection();

   // Release the connection back into the OPM
   OPM_RELEASE(connection);

   // Release the connection handle back into the OPM
   OPM_RELEASE(connectionHandle);
}//end releaseConnection


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string ConnectionSet::toString()
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
// Nested Class Definitions:
//-----------------------------------------------------------------------------

