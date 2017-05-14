/******************************************************************************
* 
* File name:   DataManager.h 
* Subsystem:   Platform Services 
* Description: Wrapper class to abstract out the details of database interaction.
*               
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DATA_MGR_H_
#define _PLAT_DATA_MGR_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <map>

#include <ace/Thread_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ConnectionSet.h"

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
 * DataManager provides an application interface for setting and retrieving
 * data to and from the persistent store.
 * <p>
 * DataManager is provided to hide some of the complexity of working with the 
 * database. It is intended to allow the database to be transparently
 * replaced by different products/mechanisms without requiring the applications
 * to do major surgery. Additionally, the DataManager defines the concurrency 
 * model for how the applications will interact with the database. For example,
 * an application program will have multiple threads; each thread requires periodic
 * access to the database. A connection set will be created for this application
 * that contains FEWER connections than the number of threads that need them. 
 * Each thread is required to reserve a database connection, use it, and then
 * release it back into the DataManager pool so that another thread can then 
 * reserve and access. Of course the number of pooled connections versus the
 * number of application threads is a systems engineering exercise and is 
 * configurable. Note that each connection is itself NOT thread safe and 
 * multiple threads should not attempt to access the same connection object
 * (DbConnectionHandle) after it has been reserved by a single thread (this
 * is typically the concurrency architecture for most database drivers). (This
 * is possible with additional mutex/concurrency mechanisms implemented by
 * the application, but it is NOT ADVISABLE, and not necessary).
 * <p>
 * The general error handling strategy for the application developer should be
 * as follows. If any errors are encountered while issuing query/update/insert/delete
 * commands or while analyzing results, the developer should release the connection
 * back into the pool and attempt to reserve another connection for use. The release
 * process includes a health check on the connection that will attempt to re-establish/
 * renew that connection in the event of a failure. 
 * <p>
 * DataManager exists as a library for each of the application processes to use
 * for managing its database connections. DataManager supports pooling of connections,
 * and connection re-establishment and retry. Because the DataManager is a bootstrap
 * requirement for the applications obtaining all of their other configuration
 * information (from the database), the DataManager reads ITS configuration from
 * a file. Furthermore, because DataManager is used by multiple processes, this
 * file must contain process-specific configuration sections to meet each
 * process's unique database connection requirements (with regards to the number
 * of connections, both local and remote, etc.)
 * <p>
 * The format of the DataManager configuration file is:
 * <pre>
 *   [Configuration]
 *   DEBUGLEVEL=5
 *   <BR><BR>
 *   [ConnectionSet Name]
 *   # Some comments
 *   # Perhaps some more comments
 *   USERID=userid
 *   PASSWORD=password (NOTE: This will need to be encrypted later...suggest MD5)
 *   DRIVERNAME=libpq
 *   CONNECTIONTYPE=LOCAL (or REMOTE)
 *   CONNECTIONSTRING="hostaddr=127.0.0.1 port=3180"
 *   NUMBERCONNECTIONS=1
 *   DSN=someDSN
 *   <BR><BR>
 *   [Next ConnectionSet Name]
 *   ...
 * </pre>
 * <p>
 * The above [Configuration] section exists in order to set DataManager-wide configuration
 * parameters. Each subsequent [ConnectionSet Name] section establishes the 
 * configuration for a pool of connections. Note that an OPM object pool is created
 * even if only 1 connection is indicated. An application process can have 
 * multiple [ConnectionSet] sections for its use in the configuration file. The
 * application process would then call DataManager::activateConnectionSet(string connectionSetName)
 * for each configured connection set that it needs to use.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class DataManager
{
   public:

      /** Virtual Destructor */
      virtual ~DataManager();

      /** 
       * Initialize the DataManager with the location of the configuration file.
       * <p>
       * Open and parse the entire configuration file. This creates a 
       * ConnectionSet object for each section in the file.
       * @returns ERROR for File IO or File format parsing errors; otherwise OK
       */
      static int initialize(string configurationFileLocation);

      /**
       * Application processes will call this method for each ConnectionSet
       * in the configuration file that they need to use. This method will
       * create the database connection pools.
       * @param connectionSetName must match a name in the configuration file
       * @returns ERROR for database connection failures; otherwise OK
       */
      static int activateConnectionSet(string connectionSetName);

      /**
       * Method for performing deActivation of the Connection Set. All connections
       * to the database are closed.
       * @param connectionSetName must match a name in the configuration file
       * @returns ERROR for database connection shutdown failures; otherwise OK
       */
      static int deActivateConnectionSet(string connectionSetName);

      /**
       * Method for performing deActivation of All Connection Sets within this
       * application process. All connections to the database are closed.
       * @returns ERROR for database connection shutdown failures; otherwise OK
       */
      static int deActivateAllConnectionSets();

      /**
       * Method to retrieve a pointer to a DbConnection from the Connection Set pool.
       * This method blocks until a DbConnection becomes available if they are
       * all currently in use.
       * @param connectionSetName must match a name in the configuration file
       * @returns NULL if the connectionSetName could not be found
       */
      static DbConnectionHandle* reserveConnection(string connectionSetName);

      /**
       * Method to retrieve a pointer to a DbConnection from the Connection Set pool.
       * This method does NOT block if there are no DbConnections available in the pool.
       * NOTE that the pools are configured not to GROW and/or SHRINK
       * @param connectionSetName must match a name in the configuration file
       * @returns NULL if no DbConnection available, or if the connectionSetName could
       *    not be found
       */
      static DbConnectionHandle* reserveConnectionNonBlocking(string connectionSetName);

      /**
       * Method to release the DbConnection back into the Connection Set pool.
       * @param connectionHandle wraps the connection to be released to the pool
       * @returns NULL if the connectionSetName could not be found
       */
      static void releaseConnection(DbConnectionHandle* connectionHandle);

   protected:

   private:

      /**
       * Create a map for storing ConnectionSets that are parsed from the DataManager
       * configuration file. Note that no attempt to allocate the pools from these
       * connections, nor is any database connection attempt made until the application
       * activates the connection set.
       **/
      typedef map<string, ConnectionSet*> ConnectionSetMap;

      /** Default Constructor */
      DataManager();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      DataManager(const DataManager& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DataManager& operator= (const DataManager& rhs);

      /**
       * Map for storing the connectionSet objects as they are created
       * during DataManager configuration file parsing.
       **/
      static ConnectionSetMap connectionSetMap_;

      /** Debug Level */
      static int debugLevel_;

      /** Non-recursive Mutex for protecting access to the ConnectionSetMap */
      static ACE_Thread_Mutex connectionSetMapMutex_;

};

#endif
