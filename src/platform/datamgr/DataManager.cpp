/******************************************************************************
*
* File name:   DataManager.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <cstring>
#include <fstream>
#include <sstream>
#include <ace/Guard_T.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "DataManager.h"
#include "ConnectionSet.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)DataManager"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// Max length of characters in the DataManager configuration file
#define MAX_CONFIG_FILE_LINE_LENGTH 256

// Debug Level
int DataManager::debugLevel_ = 0;

// Initialize the map for storing Connection Set info from the configuration file
DataManager::ConnectionSetMap DataManager::connectionSetMap_;

// Initialize the map non-recursive mutex for concurrency protection 
ACE_Thread_Mutex DataManager::connectionSetMapMutex_;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DataManager::~DataManager()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Initialize the DataManager with the location of the configuration file
// Design:
//-----------------------------------------------------------------------------
int DataManager::initialize(string configurationFileLocation)
{
   char fileBuffer[MAX_CONFIG_FILE_LINE_LENGTH];
   // Open and test the file
   ifstream configFile (configurationFileLocation.c_str());
   if (!configFile.is_open())
   {
      ostringstream ostr;
      ostr << "Unable to open DataManager Configuration file ("
           << configurationFileLocation << ")" << ends;
      STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
      return ERROR;
   }//end if

   // Read the entire file contents and parse it
   bool isDataManagerConfig = false;
   ConnectionSet* currentConnectionSet;
   // Protect multithreaded access to the connectionSetMap - return ERROR from
   // this function upon failure to get Mutex
   ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, connectionSetMapMutex_, ERROR);
   while (!configFile.eof())
   {
      configFile.getline(fileBuffer, MAX_CONFIG_FILE_LINE_LENGTH);

      // If the fileBuffer contains a [section header]
      if (strcmp(fileBuffer, "[Configuration]")==0)
      {
         // This is the dataMgr configuration section
         TRACELOG(DEBUGLOG, DATAMGRLOG, "Processing DataManager Configuration global configuration section",0,0,0,0,0,0);
         isDataManagerConfig = true;
         continue;
      }//end if
      // Section headers for Connection Set sections
      else if ((fileBuffer[0] == '[') && (fileBuffer[strlen(fileBuffer) - 1] == ']'))
      {
         // We are no longer parsing the global config section
         isDataManagerConfig = false;

         // Parse out the Connection Set Header Name. Extract the text 'between' the '[' and ']' characters.
         string tmpString(fileBuffer);
         tmpString = (tmpString.substr(tmpString.find('[') + 1, tmpString.rfind(']') - 1));

         // Create a new Connection Set to store the configuration parameters into
         currentConnectionSet = new ConnectionSet(tmpString);

         // Store this new Connection Set object in the Map using its 'Name'
         connectionSetMap_[tmpString] = currentConnectionSet;
         
         ostringstream ostr;
         ostr << "Added Map entry for connection set (" << tmpString << ")" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
      }//end else if

      // Handle each of the DataManager configuration parameters
      if (isDataManagerConfig == true)
      {
         if (strncmp(fileBuffer, "DEBUGLEVEL", strlen("DEBUGLEVEL")) == 0)
         {
            debugLevel_ = atoi(strstr(fileBuffer, "=")+1);
            TRACELOG(DEBUGLOG, DATAMGRLOG, "DataManager debug level set to %d", debugLevel_,0,0,0,0,0);
         }//end if

         continue;
      }//end else if

      // Process the variables for a particular Connection Set section
      else if (isDataManagerConfig == false)
      {
         if (strncmp(fileBuffer, "USERID", strlen("USERID")) == 0)
         {
            currentConnectionSet->setUserid(strstr(fileBuffer, "=") + 1);
            ostringstream ostr;
            ostr << "Userid is (" << (strstr(fileBuffer, "=") + 1) << ")" << ends;
            STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         }//end if
         else if (strncmp(fileBuffer, "PASSWORD", strlen("PASSWORD")) == 0)
         {
            // In the future, here we need to do some decryption to convert 
            // the encrypted password into plain text for storage. AND, this
            // this log should BE REMOVED SO AS NOT TO GIVE AWAY THE PASSWORD! 

            currentConnectionSet->setPassword(strstr(fileBuffer, "=") + 1);
            ostringstream ostr;
            ostr << "Password is (" << (strstr(fileBuffer, "=") + 1) << ")" << ends;
            STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         }//end else if
         else if (strncmp(fileBuffer, "DRIVERNAME", strlen("DRIVERNAME")) == 0)
         {
            currentConnectionSet->setDriverName(strstr(fileBuffer, "=") + 1);
            ostringstream ostr;
            ostr << "DriverName is (" << (strstr(fileBuffer, "=") + 1) << ")" << ends;
            STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         }//end else if
         else if (strncmp(fileBuffer, "CONNECTIONTYPE", strlen("CONNECTIONTYPE")) == 0)
         {
            char* connectionTypeValue = (strstr(fileBuffer, "=") + 1);
            if (strncmp(connectionTypeValue, "LOCAL", strlen("LOCAL")) == 0)
            {
               TRACELOG(DEBUGLOG, DATAMGRLOG, "Connection Type is LOCAL",0,0,0,0,0,0);
               currentConnectionSet->setConnectionType(LOCAL_CONNECTIONTYPE);
            }//end if
            else if (strncmp(connectionTypeValue, "REMOTE", strlen("REMOTE"))== 0)
            {
               TRACELOG(DEBUGLOG, DATAMGRLOG, "Connection Type is REMOTE",0,0,0,0,0,0);
               currentConnectionSet->setConnectionType(REMOTE_CONNECTIONTYPE);
            }//end else if
            else
            {
               ostringstream ostr;
               ostr << "Unknown ConnectionType is (" << (strstr(fileBuffer, "=") + 1) << ")" << ends;
               STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
            }//end else
         }//end else if
         else if (strncmp(fileBuffer, "CONNECTIONSTRING", strlen("CONNECTIONSTRING")) == 0)
         {
            // Use a string object to help parse out the contents between the quote characters
            string tmpString(strstr(fileBuffer, "=") + 1);
            tmpString = (tmpString.substr(tmpString.find('"') + 1, tmpString.rfind('"') - 1));

            currentConnectionSet->setConnectionString(tmpString.c_str());
            ostringstream ostr;
            ostr << "ConnectionString is (" << tmpString << ")" << ends;
            STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         }//end else if
         else if (strncmp(fileBuffer, "NUMBERCONNECTIONS", strlen("NUMBERCONNECTIONS")) == 0)
         {
            currentConnectionSet->setNumberConnections(atoi(strstr(fileBuffer, "=") + 1));
            TRACELOG(DEBUGLOG, DATAMGRLOG, "NumberConnections is (%d)", atoi(strstr(fileBuffer, "=") + 1),0,0,0,0,0);
         }//end else if
         else if (strncmp(fileBuffer, "DSN", strlen("DSN")) == 0)
         {
            currentConnectionSet->setDSN(strstr(fileBuffer, "=") + 1);
            ostringstream ostr;
            ostr << "DSN is (" << (strstr(fileBuffer, "=") + 1) << ")" << ends;
            STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         }//end else if  
      }//end if
   }//end while

   // close the file
   configFile.close();

   return OK; 
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Application processes will call this method for each ConnectionSet
//              in the configuration file that they need to use
// Design:
//-----------------------------------------------------------------------------
int DataManager::activateConnectionSet(string connectionSetName)
{
   ConnectionSetMap::const_iterator connectionSetIterator;
   ConnectionSetMap::const_iterator endIterator;

   // Protect multithreaded access to the connectionSetMap - return ERROR from
   // this function upon failure to get access to Mutex
   ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, connectionSetMapMutex_, ERROR);

   connectionSetIterator = connectionSetMap_.begin();
   endIterator = connectionSetMap_.end();

   //loop through the map look for the Connection Set that matches the connectionSetName
   while (connectionSetIterator != endIterator)
   {
      string tempName = connectionSetIterator->first;
      if (connectionSetName == tempName)
      {
         ostringstream ostr;
         ostr << "Performing connection activation for connection set (" 
              << connectionSetIterator->first << ")" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         // When we find the correct Connection Set call the activate method
         if (connectionSetIterator->second->performActivation() == ERROR)
         {
            ostringstream ostr;
            ostr << "Error performing connection activation for connection set ("
                 << connectionSetIterator->first << ")" << ends;
            STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
            return ERROR;
         }//end if
         return OK;
      }//end if
      connectionSetIterator++;
   }//end while
   //couldn't find it, so return ERROR
   ostringstream ostr;
   ostr << "Could not find connection set for activate (" << connectionSetName << ")" << ends;
   STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
   return ERROR;
}//end activateConnectionSet


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Application processes will call this method for a ConnectionSet
//              if they need to deactivate it for some reason (perhaps for
//              graceful shutdown)
// Design:
//-----------------------------------------------------------------------------
int DataManager::deActivateConnectionSet(string connectionSetName)
{
   ConnectionSetMap::const_iterator connectionSetIterator;
   ConnectionSetMap::const_iterator endIterator;

   // Protect multithreaded access to the connectionSetMap
   ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, connectionSetMapMutex_, ERROR);

   connectionSetIterator = connectionSetMap_.begin();
   endIterator = connectionSetMap_.end();

   //loop through the map look for the Connection Set that matches the connectionSetName
   while (connectionSetIterator != endIterator)
   {
      string tempName = connectionSetIterator->first;
      if (connectionSetName == tempName)
      {
         ostringstream ostr;
         ostr << "Performing connection deactivation for connection set ("
              << connectionSetIterator->first << ")" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         // When we find the correct Connection Set call the deactivate method
         connectionSetIterator->second->performDeactivation();
         return OK;
      }//end if
      connectionSetIterator++;
   }//end while
   //couldn't find it, so return ERROR
   ostringstream ostr;
   ostr << "Could not find connection set for deactivate (" << connectionSetName << ")" << ends;
   STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
   return ERROR;
}//end deActivateConnectionSet


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Application processes will call this method to deActivate ALL
//              ConnectionSets in this process.
// Design:
//-----------------------------------------------------------------------------
int DataManager::deActivateAllConnectionSets()
{
   ConnectionSetMap::const_iterator connectionSetIterator;
   ConnectionSetMap::const_iterator endIterator;
   
   // Protect multithreaded access to the connectionSetMap
   ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, connectionSetMapMutex_, ERROR);

   connectionSetIterator = connectionSetMap_.begin();
   endIterator = connectionSetMap_.end();

   //loop through the map look for the Connection Set that matches the connectionSetName
   while (connectionSetIterator != endIterator)
   {
      string tempName = connectionSetIterator->first;
      ostringstream ostr;
      ostr << "Performing connection deactivation for connection set ("
           << connectionSetIterator->first << ")" << ends;
      STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
      // When we find the correct Connection Set call the deactivate method
      connectionSetIterator->second->performDeactivation();
      connectionSetIterator++;
   }//end while
   return OK;
}//end deActivateAllConnectionSets


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to retrieve a pointer to a DbConnection from the Connection Set pool
// Design:
//-----------------------------------------------------------------------------
DbConnectionHandle* DataManager::reserveConnection(string connectionSetName)
{
   ConnectionSetMap::const_iterator connectionSetIterator;
   ConnectionSetMap::const_iterator endIterator;

   // Protect multithreaded access to the connectionSetMap
   ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, connectionSetMapMutex_, NULL);

   connectionSetIterator = connectionSetMap_.begin();
   endIterator = connectionSetMap_.end();

   //loop through the map look for the Connection Set that matches the connectionSetName
   while (connectionSetIterator != endIterator)
   {
      string tempName = connectionSetIterator->first;
      if (connectionSetName == tempName)
      {
         ostringstream ostr;
         ostr << "Reserving a connection from connection set ("
              << connectionSetIterator->first << ")" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         // When we find the correct Connection Set, reserve a connection and return it
         return connectionSetIterator->second->reserveConnection();
      }//end if
      connectionSetIterator++;
   }//end while
   //couldn't find it, so return NULL 
   ostringstream ostr;
   ostr << "Could not find connection set for reserve (" << connectionSetName << ")" << ends;
   STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
   return NULL;
}//end reserveConnection


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to retrieve a pointer to a DbConnection from the Connection Set pool
// Design: This Method will not block if objects are not available, but instead
//         return NULL.
//-----------------------------------------------------------------------------
DbConnectionHandle* DataManager::reserveConnectionNonBlocking(string connectionSetName)
{
   ConnectionSetMap::const_iterator connectionSetIterator;
   ConnectionSetMap::const_iterator endIterator;

   // Protect multithreaded access to the connectionSetMap
   ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, connectionSetMapMutex_, NULL);

   connectionSetIterator = connectionSetMap_.begin();
   endIterator = connectionSetMap_.end();

   //loop through the map look for the Connection Set that matches the connectionSetName
   while (connectionSetIterator != endIterator)
   {
      string tempName = connectionSetIterator->first;
      if (connectionSetName == tempName)
      {
         ostringstream ostr;
         ostr << "Reserving a connection from connection set ("
              << connectionSetIterator->first << ")" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         // When we find the correct Connection Set, reserve (non-blocking) a connection and return it
         // This may return NULL if no connections available
         return connectionSetIterator->second->reserveConnectionNonBlocking();
      }//end if
      connectionSetIterator++;
   }//end while
   //couldn't find it, so return NULL
   ostringstream ostr;
   ostr << "Could not find connection set for non-blocking reserve (" << connectionSetName << ")" << ends;
   STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
   return NULL;
}//end reserveConnectionNonBlocking


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to release the DbConnection back into the Connection Set pool
// Design:
//-----------------------------------------------------------------------------
void DataManager::releaseConnection(DbConnectionHandle* connectionHandle)
{
   ConnectionSetMap::const_iterator connectionSetIterator;
   ConnectionSetMap::const_iterator endIterator;

   // Protect multithreaded access to the connectionSetMap
   ACE_GUARD(ACE_Thread_Mutex, guard, connectionSetMapMutex_);

   connectionSetIterator = connectionSetMap_.begin();
   endIterator = connectionSetMap_.end();

   // Get the owning connection Set Name that is stored inside the connection handle
   string connectionSetName = connectionHandle->getConnectionSetName();

   //loop through the map look for the Connection Set that matches the connectionSetName
   while (connectionSetIterator != endIterator)
   {
      string tempName = connectionSetIterator->first;
      if (connectionSetName == tempName)
      {
         ostringstream ostr;
         ostr << "Releasing a connection back into connection set (" << connectionSetName << ")" << ends;
         STRACELOG(DEBUGLOG, DATAMGRLOG, ostr.str().c_str());
         // When we find the correct Connection Set, release the connectionHandle into it
         connectionSetIterator->second->releaseConnection(connectionHandle);
         return;
      }//end if
      connectionSetIterator++;
   }//end while
   //couldn't find it, so return
   ostringstream ostr;
   ostr << "Memory Leak: Could not find connection set for release (" << connectionSetName << ")" << ends;
   STRACELOG(ERRORLOG, DATAMGRLOG, ostr.str().c_str());
   return;
}//end releaseConnection


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Default Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
DataManager::DataManager()
{
}//end constructor


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

