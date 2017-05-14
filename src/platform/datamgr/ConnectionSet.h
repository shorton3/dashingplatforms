/******************************************************************************
* 
* File name:   ConnectionSet.h 
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

#ifndef _PLAT_DATAMGR_CONNECTION_SET_H_
#define _PLAT_DATAMGR_CONNECTION_SET_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DbConnectionHandle.h"

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
 * ConnectionSet handles activation and deactivation of the database
 * connections (including connection establishment and error handling).
 * <p>
 * ConnectionSet also interacts with OPM for connection pooling.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

typedef enum
{
   UNKNOWN_CONNECTIONTYPE = 0,
   LOCAL_CONNECTIONTYPE,
   REMOTE_CONNECTIONTYPE,
   MAX_CONNECTIONTYPE
} ConnectionSetType;


class ConnectionSet
{
   public:

      /** Constructor */
      ConnectionSet(string connectionSetName);

      /** Virtual Destructor */
      virtual ~ConnectionSet();

      /** Set the Userid associated with the connection */
      void setUserid(string userid);

      /** Set the Password associated with the connection */
      void setPassword(string password);

      /** Set the Driver Name associated with the connection */
      void setDriverName(string driverName);

      /** Set the Connection Type associated with the connection */
      void setConnectionType(ConnectionSetType connectionType);

      /** Set the Connection String associated with the connection */
      void setConnectionString(string connectionString);

      /** Set the number of connections */
      void setNumberConnections(int numberConnections);

      /** Set the DSN */
      void setDSN(string dsn);

      /** 
       * Perform activation for the connection set.
       * This includes connection establishment, error handling, and creation
       * of the OPM pool for storing the active connections.
       * @returns ERROR upon failure; otherwise OK
       */
      int performActivation();

      /**
       * Perform deactivation of the connection set.
       * This should include connection tear down, error handling, and destruction
       * of the OPM pool for the connections, but currently there is no OPM support
       * for deallocating the pool of objects. We can loop through all of the 'Available'
       * connection objects and perform a disconnect/shutdown. HOWEVER, this does
       * assume that the application has released all of the connection objects back
       * into the pool already (so that we can re-reserve them here).
       * @returns ERROR upon failure; otherwise OK
       */
      int performDeactivation();

      /**
       * Method to retrieve a pointer to a DbConnection from the Connection Set pool.
       * This method blocks until a DbConnection becomes available if they are
       * all currently in use. This method is called from the similarly named
       * static method in the DataManager.
       */
      DbConnectionHandle* reserveConnection();

      /**
       * Method to retrieve a pointer to a DbConnection from the Connection Set pool.
       * This method does NOT block if there are no DbConnections available in the pool.
       * NOTE that the pools are configured not to GROW and/or SHRINK. This method
       * is called from the similarly named static method in the DataManager.
       * @returns NULL if no DbConnection available
       */
      DbConnectionHandle* reserveConnectionNonBlocking();

      /**
       * Method to release the DbConnection back into the Connection Set pool.
       * This method is called from the similarly named static method in the DataManager.
       */
      void releaseConnection(DbConnectionHandle* connectionHandle);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ConnectionSet(const ConnectionSet& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ConnectionSet& operator= (const ConnectionSet& rhs);

      /** Connection Set Name that was parsed from the Configuration file Header section */
      string connectionSetName_; 

      /** Userid for the connections */
      string userid_;

      /** Password for the connections */
      string password_;

      /** Driver Name to use for the connections. This determines which type of
          database we are connecting to */
      string driverName_;

      /** Connection type */
      ConnectionSetType connectionType_;

      /** Connection String */
      string connectionString_;

      /** Number of Connections */
      int numberConnections_;

      /** Database DSN */
      string dsn_;

      /** OPM Pool Id for the pool that manages this Connection Set's connection objects */
      int connectionObjectPoolId_;

      /** OPM Pool Id for the pool that manages ALL of the ConnectionHandle objects.
          (for all pools in this process */
      static int handleObjectPoolId_;

};

#endif
