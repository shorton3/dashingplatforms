/******************************************************************************
*
* File name:   ClientAgent.cpp
* Subsystem:   EMS
* Description: CORBA Enabled Agent for receiving and processing client commands
*              from the various client interfaces.
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

#include <iostream>

#include <ace/Get_Opt.h>
#include <ace/Sig_Adapter.h>
#include <ace/Select_Reactor.h>

#include <tao/ORB_Core.h>
#include <orbsvcs/CosNamingC.h>
#include <tao/Utils/ORB_Manager.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ClientAgent.h"

#include "platform/datamgr/DataManager.h"
#include "platform/datamgr/DbConnectionHandle.h"
#include "platform/common/ConnectionSetNames.h"

#include "platform/opm/OPM.h"

#include "platform/common/MailboxNames.h"
#include "platform/common/MessageIds.h"
#include "platform/msgmgr/DistributedMailbox.h"
#include "platform/msgmgr/MailboxProcessor.h"
#include "platform/msgmgr/MessageFactory.h"
#include "platform/msgmgr/TimerMessage.h"
#include "platform/messages/AlarmEventMessage.h"

#include "platform/logger/Logger.h"

#include "platform/threadmgr/ThreadManager.h"

#include "platform/utilities/SystemInfo.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Client Agent"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

// Static singleton instance
ClientAgent::ClientAgent* ClientAgent::clientAgent_ = NULL;

// Static Select Reactor instance
ACE_Reactor* ClientAgent::selectReactor_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ClientAgent::ClientAgent()
               :clientAgentMailbox_(NULL),
                messageHandlerList_(NULL),
                sessionImplPtr_(NULL),
                platformConfigImplPtr_(NULL),
                localNEID_("000000000"),
                eventReportFileName_(""),
                fileCheckCounter_(0),
                numberKeptEventFiles_(0),
                sizeKeptEventFiles_(0)
{
   // Populate the static single instance
   clientAgent_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ClientAgent::~ClientAgent()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Catch the shutdown signal and prepare to graceful stop
// Design:
//-----------------------------------------------------------------------------
void ClientAgent::catchShutdownSignal()
{
   TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Requested ClientAgent shutdown",0,0,0,0,0,0);
   ClientAgent::getInstance()->shutdown();
}//end catchShutdownSignal


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Retrieve the static singleton instance
// Design:
//-----------------------------------------------------------------------------
ClientAgent* ClientAgent::getInstance()
{
   if (clientAgent_ == NULL)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Singleton instance is NULL",0,0,0,0,0,0);
   }//end if
   return clientAgent_;
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Gracefully shutdown the Application and all of its child threads
// Design:
//-----------------------------------------------------------------------------
void ClientAgent::shutdown()
{
   // Cancel the outstanding timers


   // Shutdown the Corba ORB and the event processing loop
   //TAO_ORB_Core_instance()->orb()->destroy();

   // Here, we need to post a message to the local mailbox in order for the mailbox
   // to be deactivated and released from the owner thread.
   //
   // Deactivate the Mailbox
   //if (clientAgentMailbox_->deactivate() == ERROR)
   //{
   //   TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error deactivating Client Agent Mailbox for shutdown",0,0,0,0,0,0);
   //}//end if
   //
   // Release the Mailbox for cleanup
   //clientAgentMailbox_->release();

   // Shutdown the reactor
   selectReactor_->owner (ACE_Thread::self ());
   selectReactor_->end_reactor_event_loop();

   // Exit this process
   ACE::terminate_process(getpid());
}//end shutdown


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method starts the reactor thread needed for signal handling, etc.
// Design:
//-----------------------------------------------------------------------------
void ClientAgent::startReactor()
{
   // Set Reactor thread ownership
   selectReactor_->owner (ACE_Thread::self ());
   // Start the reactor processing loop
   while (selectReactor_->reactor_event_loop_done () == 0)
   {
      int result = selectReactor_->run_reactor_event_loop ();

      char errorBuff[200];
      char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
      if (resultStr == NULL)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str());
   }//end while
}//end startReactor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialization method
// Design:
//-----------------------------------------------------------------------------
int ClientAgent::initialize(bool dottedDecimalIORFlag, bool debugFlag, string& debugLevel,
     string& debugFilename, string& endPointString, string& initRef, string& defaultInitRef)
{
   char* exceptionMessage = NULL;

   // Initialize the DataManager and activate database connections first!
   if (DataManager::initialize("./DataManager.conf") == ERROR)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error initializing Data manager",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Successfully initialized Data Manager",0,0,0,0,0,0);
   }//end else

   // Retrieve our local NEID from the DataManager and store it
   localNEID_ = SystemInfo::getLocalNEID();

   // Activate the Logger Connection Set
   if (DataManager::activateConnectionSet(CLIENT_AGENT_CONNECTION) == ERROR)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error activating Logger ConnectionSet",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Successfully activated Logger ConnectionSet",0,0,0,0,0,0);

      // Use a database connection to read the loglevels from the database and set them into shared memory
      DbConnectionHandle* connectionHandle = DataManager::reserveConnection(CLIENT_AGENT_CONNECTION);
      if (connectionHandle == NULL)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error reserving connection",0,0,0,0,0,0);
         return ERROR;
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Successfully reserved Logger ConnectionSet Connection",0,0,0,0,0,0);
      }//end else

      // Run a query against the LogLevels table
      string logLevelQuery = "Select SubSystem,LogLevel from platform.LogLevels where NEID='" + localNEID_ + "'";

      // Execute the query against the database
      if (connectionHandle->executeCommand(logLevelQuery) == ERROR)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Log Level query execution failed",0,0,0,0,0,0);
         connectionHandle->closeCommandResult();
         DataManager::releaseConnection(connectionHandle);
         return ERROR;
      }//end if

      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Preparing to update shared memory log levels with database values",0,0,0,0,0,0);   

      int columnCount = connectionHandle->getColumnCount();
      int rowCount = connectionHandle->getRowCount();
      // Do some checks to see if the number of columns returned is zero! This indicates a schema error!
      if (columnCount == 0)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "No columns returned in result set, possible schema error",0,0,0,0,0,0);
         connectionHandle->closeCommandResult();
         DataManager::releaseConnection(connectionHandle);
         return ERROR;
      }//end if
      else if (rowCount == 0)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "No rows returned in result set, default datafill missing",0,0,0,0,0,0);
         connectionHandle->closeCommandResult();
         DataManager::releaseConnection(connectionHandle);
         return ERROR;
      }//end if
      else
      {
         // Don't assume the subsystem type is field 0 and severity Level is field 1 
         int subSystemColumnIndex = connectionHandle->getColumnIndex("SubSystem");
         int severityColumnIndex = connectionHandle->getColumnIndex("LogLevel");

         for (int row = 0; row < rowCount; row++)
         {
            Logger::setSubsystemLogLevel((LogSubSystemType)connectionHandle->getShortValueAt(row,subSystemColumnIndex), 
               (LogSeverityType)connectionHandle->getShortValueAt(row,severityColumnIndex));
         }//end for
      }//end else

      // Close the results, release the connection
      connectionHandle->closeCommandResult();
      DataManager::releaseConnection(connectionHandle);
   }//end else

   // Create the Distributed Mailbox Address
   MailboxAddress distributedAddress;
   distributedAddress.locationType = DISTRIBUTED_MAILBOX;
   distributedAddress.mailboxName = CLIENT_AGENT_MAILBOX_NAME;
   distributedAddress.inetAddress.set(CLIENT_AGENT_MAILBOX_PORT, LOCAL_IP_ADDRESS);
   distributedAddress.neid = SystemInfo::getLocalNEID();

   // Setup the distributed Mailbox
   if (!setupMailbox(distributedAddress))
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Unable to setup distributed mailbox",0,0,0,0,0,0);
      return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "ClientAgent distributed mailbox setup successful",0,0,0,0,0,0);
   }//end else

//TODO Move this hardcoded stuff elsewhere and set using command line parameters
   // Do some hardcoded initialization in order to write Event Reports to a file
   TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Opening /tmp/platform.EventReports for event reports",0,0,0,0,0,0);
   eventReportFileName_ = "/tmp/platform.EventReports";
   numberKeptEventFiles_ = 5;
   sizeKeptEventFiles_ = 10000000; // 10 meg for now
   eventReportFileStream_.open (eventReportFileName_.c_str(), ios_base::out | ios_base::app);
   if (eventReportFileStream_.fail())
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Failed to open Event Reports file",0,0,0,0,0,0);
   }//end if
//End TODO

   // Create a new ACE_Select_Reactor for the signal handling, etc to use
   selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create an adapter to shutdown this service and all children
   signalAdapter_ = new ACE_Sig_Adapter((ACE_Sig_Handler_Ex) ClientAgent::catchShutdownSignal);

   // Specify which types of signals we want to trap
   signalSet_.sig_add (SIGINT);
   signalSet_.sig_add (SIGQUIT);
   signalSet_.sig_add (SIGTERM);

   // Register ourselves to receive signals so we can shut down gracefully.
   if (selectReactor_->register_handler (signalSet_, signalAdapter_) == ERROR)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error registering for OS signals",0,0,0,0,0,0);
   }//end if

   // Startup the Reactor processing loop in a dedicated thread. Here, returns the OS assigned
   // unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)ClientAgent::startReactor, (void*) 0, "ClientAgentReactor", true);

   try
   {
      exceptionMessage = "While ORB Manager init";

      // Build the Orb.init arguments. This is necessary since there is some bug that prevents
      // the Orb from initializing properly after the arguments have been run through ACE::Get_Opt.
      int numbArgs = 0;
      char** orbArgs = new char *[20];
      orbArgs[numbArgs++] = "ClientAgent";
      orbArgs[numbArgs++] = "-ORBDottedDecimalAddresses";
      if (dottedDecimalIORFlag == true)
      {
         TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Restricting IORs to use IP Addresses only",0,0,0,0,0,0);
         orbArgs[numbArgs++] = "1";
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "IORs allowed to contain Hostnames for DNS resolution",0,0,0,0,0,0);
         orbArgs[numbArgs++] = "0";
      }//end else
      if (endPointString.empty() == false)
      {
         ostringstream ostr;
         ostr << "Orb instructed to use endpoints: " << endPointString << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
         orbArgs[numbArgs++] = "-ORBListenEndpoints";
         orbArgs[numbArgs++] = (char*)endPointString.c_str();
      }//end if
      if (initRef.empty() == false)
      {
         ostringstream ostr;
         ostr << "Orb instructed to use initial reference: " << initRef << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
         orbArgs[numbArgs++] = "-ORBInitRef";
         orbArgs[numbArgs++] = (char*)initRef.c_str();
      }//end if
      if (defaultInitRef.empty() == false)
      {
         ostringstream ostr;
         ostr << "Orb instructed to use default initial reference: " << defaultInitRef << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
         orbArgs[numbArgs++] = "-ORBDefaultInitRef";
         orbArgs[numbArgs++] = (char*)defaultInitRef.c_str();
      }//end if
      if (debugFlag == true)
      {
         ostringstream ostr;
         ostr << "Orb internal debug logs enabled to level " << debugLevel << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
         orbArgs[numbArgs++] = "-ORBDebug";
         orbArgs[numbArgs++] = "-ORBDebugLevel";
         orbArgs[numbArgs++] = (char*)debugLevel.c_str();
         if (debugFilename.empty() == false)
         {
            ostringstream ostr;
            ostr << "Orb internal debug logs redirected to file " << debugFilename << ends;
            STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
            orbArgs[numbArgs++] = "-ORBLogFile";
            orbArgs[numbArgs++] = (char*)debugFilename.c_str();
         }//end if
      }//end if

      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Calling orb init with %d arguments",numbArgs,0,0,0,0,0);

      // Initialize the Orb Manager - for GIOP 1.1, try: -ORBEndPoint iiop://1.1@<host>:<port>
      if (orbManager_.init (numbArgs, orbArgs) == ERROR)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error initializing the orb manager",0,0,0,0,0,0);
         return ERROR;
      }//end if

      // Obtain the RootPOA. It is created in holding state when the Orb is created, so
      // we need to activate it.
      CORBA::Object_var obj = orbManager_.orb()->resolve_initial_references("RootPOA");

      // Get the POA_var object from Object_var.
      exceptionMessage = "While narrowing the root poa";
      PortableServer::POA_var rootPOA = PortableServer::POA::_narrow(obj.in());

      // Get the POAManager of the RootPOA.
      exceptionMessage = "While getting the POA Manager";
      PortableServer::POAManager_var poaManager = rootPOA->the_POAManager();

      // Activate the POA Manager before activating any sub-objects
      exceptionMessage = "While activating the POA Manager";
      poaManager->activate();

      // Create the IDL-specific objects and register them with the POA
      exceptionMessage = "While creating IDL objects";
      createIDLObjects(rootPOA); 

      // Perform initial resolution of the Naming Service. Then, bind our top level
      // objects into the Naming Service.
      exceptionMessage = "While initializing the Naming Service bindings";
      initNamingService();

      // Resolve client interface objects from the Naming Service
      exceptionMessage = "While resolving client objects from the Naming Service";
      resolveClientObjects();
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      ostringstream ostr;
      ostr << "Application Level Exception - " << exceptionMessage
           << ". Corba Orb Level Exception - " << exception._info() << ends;
      STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str());
      return ERROR;
   }//end catch

   // NOTE, TBD: We have not done a good job on retry semantics. If any of the above
   // throws an exception, then we simply error-out and the Orb will not get started.
   // Suggestion to improve above: Perhaps we should not return Errors in the initNamingService
   // and resolveClientObjects methods as these depend on external components. We need
   // some mechanism to periodically retry (or at least when we need / on application demand)
   // for those methods.

   // Start the TAO Orb event loop in its own dedicated thread. From there, 
   // TAO will spawn additional threads and will handle its own thread / connection
   // management. Here, returns OS assigned unique thread Id
   // NOTE: In the examples, the Orb thread is created with THR_DETACHED thread option
   // so the OS will reclaim its resources and exit status is not known, but let's
   // -undo- that and make it THR_JOINABLE:
   ThreadManager::createThread((ACE_THR_FUNC)ClientAgent::runOrb, (void*) 0, "ClientAgentOrb", true);

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Create the IDL-specific object interfaces
// Design:
//-----------------------------------------------------------------------------
int ClientAgent::createIDLObjects(PortableServer::POA_var& rootPOA)
{
   char* exceptionMessage = NULL;
   try
   {
      // Create an instance of the Session interface object. This object is
      // responsible for monitoring of agent/client communication
      ACE_NEW_RETURN (sessionImplPtr_, session_Session_I_i(), ERROR);

      PortableServer::ObjectId_var sessionOid = PortableServer::string_to_ObjectId ("AgentSession");

      exceptionMessage = "While activating Agent Session";
      rootPOA->activate_object_with_id (sessionOid.in(), sessionImplPtr_);

      // For debugging, here we get an Object reference for session object.
      // This verifies that the ORB is correctly managing the object.
      exceptionMessage = "While Session::_this";
      session::Session_I* sessionVar = sessionImplPtr_->_this();

      // Stringify the object reference and print it out
      exceptionMessage = "While object_to_string";
      CORBA::String_var sessionIOR = this->orbManager_.orb()->object_to_string (sessionVar);

      // Print the IOR
      ostringstream ostr;
      ostr << "Agent Session IOR is " << sessionIOR.in() << ends;
      STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());

      // Create an instance of the platformConfig interface object. This object is
      // responsible for handling configuration management requests
      ACE_NEW_RETURN (platformConfigImplPtr_, platformConfig_platformConfig_I_i(), ERROR);

      PortableServer::ObjectId_var platformConfigOid = PortableServer::string_to_ObjectId("PlatformConfig");
 
      exceptionMessage = "While activating Platform Config";
      rootPOA->activate_object_with_id (platformConfigOid.in(), platformConfigImplPtr_);

      // For debugging, here we get an Object reference for the platformConfig object.
      // This verifies that the ORB is correctly managing the object.
      exceptionMessage = "While PlatformConfig::_this";
      platformConfig::platformConfig_I* platformConfigVar = platformConfigImplPtr_->_this();

      // Stringify the object reference and print it out
      exceptionMessage = "While object_to_string";
      CORBA::String_var platformConfigIOR = this->orbManager_.orb()->object_to_string (platformConfigVar);

      // Print the IOR
      ostringstream ostr2;
      ostr2 << "Platform Config IOR is " << platformConfigIOR.in() << ends;
      STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());

      // Create an instance of the alarms interface object. This object is
      // responsible for handling alarm management requests
      ACE_NEW_RETURN (alarmImplPtr_, alarms_alarmInterface_i(), ERROR);

      PortableServer::ObjectId_var alarmOid = PortableServer::string_to_ObjectId("Alarms");

      exceptionMessage = "While activating Alarms";
      rootPOA->activate_object_with_id (alarmOid.in(), alarmImplPtr_);

      // For debugging, here we get an Object reference for the alarms object.
      // This verifies that the ORB is correctly managing the object.
      exceptionMessage = "While alarms::_this";
      alarms::alarmInterface* alarmVar = alarmImplPtr_->_this();

      // Stringify the object reference and print it out
      exceptionMessage = "While object_to_string";
      CORBA::String_var alarmIOR = this->orbManager_.orb()->object_to_string (alarmVar);

      // Print the IOR
      ostringstream ostr3;
      ostr2 << "Alarms IOR is " << alarmIOR.in() << ends;
      STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());

      // Create additional IDL interface objects here...


   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      ostringstream ostr;
      ostr << "Application Level Exception - " << exceptionMessage
           << ". Corba Orb Level Exception - " << exception._info() << ends;
      STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str());
      return ERROR;
   }//end catch
   return OK;
}//end createIDLObjects


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform initial resolution of the Naming Service. Then, bind
//   our top level objects into the Naming Service. 
// Design:
//-----------------------------------------------------------------------------
int ClientAgent::initNamingService()
{
   const char *exceptionMessage = NULL;
   try
   {
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Attempting to resolve the Naming Service",0,0,0,0,0,0);

      // Resolve the Naming Service generic reference - Success here depends on the value of
      // -ORBInitRef (or ORBDefaultInitRef) passed in to Orb.init from the argc/argv options to
      // specify the corbaloc/iiop target for Interoperable Naming Service lookup. The only 
      // alternative to doing it this way is to actually have the root IOR of the Naming
      // Service (possibly exchanged via a URL or via database replication, file exchange, or whatever)
      exceptionMessage = "While resolving Naming Service";
      CORBA::ORB_ptr orbPtr = TAO_ORB_Core_instance()->orb();
      CORBA::Object_var namingObj = orbPtr->resolve_initial_references ("NameService");

      // Check to see if the Naming Service reference is null
      if (CORBA::is_nil (namingObj.in ()))
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Unable to resolve Naming Service",0,0,0,0,0,0);
         // Need to implement retry semantics here!!


         return ERROR;
      }//end if

      exceptionMessage = "While narrowing Naming Service context";
      namingContextVar_ = CosNaming::NamingContext::_narrow (namingObj.in());

      // Check to see if there was a problem performing narrow
      if (CORBA::is_nil (namingContextVar_.in()))
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Naming Service Context reference is null",0,0,0,0,0,0);
         // Implement retry semantics here (if necessary)!!

         return ERROR;
      }//end if

      // Bind the Session IDL object into the naming service. This is the top level
      // interface from which all other interfaces should be retrieved. So, we should not
      // need to bind other interface objects to the Naming Service later.
      CosNaming::Name sessionName (1);
      sessionName.length (1);
      // TBD: This should be replaced with a unique identifier that distinguishes this
      // Client Agent from other possible client agents (or at least the redundant one). Needs discussion.
      sessionName[0].id = CORBA::string_dup ("AgentSession");
      sessionName[0].kind = CORBA::string_dup ("server");
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Client Agent Naming Service Bind Name is AgentSession", 0,0,0,0,0,0);

      exceptionMessage = "While using Session _this";
      session::Session_I* sessionVar = sessionImplPtr_->_this();

      exceptionMessage = "While rebinding Session";
      namingContextVar_->rebind (sessionName, sessionVar);

      // For now, Bind the PlatformConfig configuration management interface object into the
      // Naming Service. Later, this can be modified so that all 'sub-interfaces' are retrievable
      // via the session interface by introspection.
      CosNaming::Name platformConfigName (1);
      platformConfigName.length (1);
      platformConfigName[0].id = CORBA::string_dup ("PlatformConfig");
      platformConfigName[0].kind = CORBA::string_dup ("server");

      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Client Agent binding PlatformConfig to the Naming Service",0,0,0,0,0,0);

      exceptionMessage = "While using PlatformConfig _this";
      platformConfig::platformConfig_I* platformConfigVar = platformConfigImplPtr_->_this();

      exceptionMessage = "While rebinding PlatformConfig";
      namingContextVar_->rebind (platformConfigName, platformConfigVar);

      // For now, Bind the Alarms management interface object into the Naming Service.
      // Later, this can be modified so that all 'sub-interfaces' are retrievable
      // via the session interface by introspection.
      CosNaming::Name alarmName (1);
      alarmName.length (1);
      alarmName[0].id = CORBA::string_dup ("Alarms");
      alarmName[0].kind = CORBA::string_dup ("server");

      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Client Agent binding Alarms Interface to the Naming Service",0,0,0,0,0,0);

      exceptionMessage = "While using Alarms _this";
      alarms::alarmInterface* alarmVar = alarmImplPtr_->_this();

      exceptionMessage = "While rebinding Alarms";
      namingContextVar_->rebind (alarmName, alarmVar);
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      ostringstream ostr;
      ostr << "Application Level Exception - " << exceptionMessage
           << ". Corba Orb Level Exception - " << exception._info() << ends;
      STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str());
      return ERROR;
   }//end catch
   return OK;
}//end initNamingService


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Resolve/retrieve our clients' top level interface objects from 
//   the Naming Service
// Design:
//-----------------------------------------------------------------------------
int ClientAgent::resolveClientObjects()
{
   const char* exceptionMessage = NULL;
   try
   {
      // Resolve the Clients' Session reference which is the top level object reference. From
      // there we can get references to all of the other remote interfaces
      CosNaming::Name clientSessionContextName (1);
      clientSessionContextName.length (1);
      // NOTE, TBD: This needs to be replaced with a unique distinguishing string for
      // each client. For now, we only support 1 client!!!!
      clientSessionContextName[0].id = CORBA::string_dup ("ClientSession");

      if (CORBA::is_nil (namingContextVar_.in()))
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Naming Service Context reference is NULL",0,0,0,0,0,0);
         return ERROR;
      }//end if

      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Resolving Clients' Session Interfaces",0,0,0,0,0,0);
      exceptionMessage = "While resolving clients' session interfaces";
      CORBA::Object_var clientSessionVar = namingContextVar_->resolve(clientSessionContextName);

      exceptionMessage = "While narrowing the NMS Session Interface";
      session_I_ptr_ = session::Session_I::_narrow (clientSessionVar.in());

      if (CORBA::is_nil (session_I_ptr_) || session_I_ptr_->_non_existent())
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Failed to resolve Clients' Session Interfaces in Naming Service",0,0,0,0,0,0);
         return ERROR;
      }//end if

      // Display list of Clients and their IORs
      ostringstream ostr;
      ostr << "IOR of the Client Session object is " 
           << this->orbManager_.orb()->object_to_string(session_I_ptr_) << ends;
      STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());

      // Set the reference of the Client Session object into our own Session object
      // as the 'Associated Session' -- NOTE: This is a cheat to satisfy the IDL as it
      // only supports storing 1 client reference in the IDL. We need a way to store
      // references to all attached clients, but since we don't use it for now.....
      sessionImplPtr_->setAssociatedSession(session_I_ptr_);


      // Resolve other Client interfaces here...


   }//end try
   //catch NamingContext error
   catch (CosNaming::NamingContext::NotFound notFoundException)
   {
      ostringstream ostr;
      ostr << "CosNaming Naming Service Context not found " << exceptionMessage 
           << ": " <<  notFoundException._info() << ends;
      STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str()); 
      return ERROR;
   }//end catch
   catch (CosNaming::NamingContext::CannotProceed cannotProceed)
   {
      ostringstream ostr;
      ostr << "CosNaming Cannot proceed user exception " << exceptionMessage
           << ": " << cannotProceed._info() << ends; 
      STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str());
      return ERROR;
   }//end catch
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      ostringstream ostr;
      ostr << "Application Level Exception - " << exceptionMessage
           << ". Corba Orb Level Exception - " << exception._info() << ends;
      STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str());
      return ERROR;
   }//end catch
   return OK;
}//end resolveClientObjects


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the mailbox processing loop
// Design:
//-----------------------------------------------------------------------------
void ClientAgent::processMailbox()
{
   // Create the Mailbox Processor
   MailboxProcessor mailboxProcessor(messageHandlerList_, *clientAgentMailbox_);

   // Activate our Mailbox (here we start receiving messages into the Mailbox queue)
   if (clientAgentMailbox_->activate() == ERROR)
   {
      // Here we need to do some better error handling. If this fails, then we did NOT
      // connect to the remote side. Applications need to retry.
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Failed to activate distributed mailbox",0,0,0,0,0,0);
   }//end if

   // Start processing messages from the Mailbox queue
   mailboxProcessor.processMailbox();
}//end processMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string ClientAgent::toString()
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
// Method Type: STATIC
// Description: Method used to start the TAO Orb in its own dedicated thread
// Design:
//-----------------------------------------------------------------------------
void ClientAgent::runOrb()
{
   // Start the orb main event loop - THIS IS AN INFINITE BLOCKING CALL
   TAO_ORB_Core_instance()->orb()->run();
}//end runOrb


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handler to Process Alarm Event MsgMgr Messages
// Design:
//-----------------------------------------------------------------------------
int ClientAgent::processAlarmEventMessage(MessageBase* message)
{
   if (message == NULL)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Received a null message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   AlarmEventMessage* alarmEventMessage = (AlarmEventMessage*) message;
 
//FOR NOW, SINCE WE ARE GOING TO MOVE THIS CODE INTO SOME OTHER CLASS... I SUGGEST MOVING THIS TO 2 NEW PROCESSES:
// EmsAlarmMgr and EmsEventReportMgr (THEY CAN EVEN HAVE SEPARATE MAILBOXES FOR SCALABILITY)!!

   // NOTE: Each of the FaultManager processes on the respective cards already
   // checks for duplicate Alarms/Clears, so we SHOULD NOT need to check in the database
   // to see if these Alarms/Clears have already happened. Therefore, we just insert/delete
   // from the Database and send to the Clients/NOCs.

   // Figure out which type of message it is (Alarm, Clear, Event Report)
   if (alarmEventMessage->getMessageType() == CLEAR_ALARM_MESSAGE)
   {
      // Get access to a database connection
      DbConnectionHandle* connectionHandle = DataManager::reserveConnection(CLIENT_AGENT_CONNECTION);
      if (connectionHandle == NULL)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error reserving connection",0,0,0,0,0,0);
         return ERROR;
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Successfully reserved Connection for AlarmEvent Management",0,0,0,0,0,0);
      }//end else

      // Issue a debug log if the Alarm is being Cleared on behalf (by proxy) of another node
      if (alarmEventMessage->getSourceAddress().neid != alarmEventMessage->getNEID())
      {
         ostringstream ostr;
         ostr << "Received Proxy Clear Alarm with id (" << alarmEventMessage->getAlarmCode()
              << ") raised on behalf of NEID (" << alarmEventMessage->getNEID() 
              << ") by NEID (" << alarmEventMessage->getSourceAddress().neid << ")" << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
      }//end if
      else
      {
         ostringstream ostr;
         ostr << "Received Clear Alarm with id (" << alarmEventMessage->getAlarmCode()
              << ") from NEID (" << alarmEventMessage->getNEID() << ")" << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
      }//end else 

      // Build an update statement to mark the alarm record as CLEARED in the system-wide outstanding
      // alarms table. We do not want to delete it since we must maintain a history of acknowledgements
      // NOTE: We need a strategy for when this table gets cleaned UP!!
      ostringstream outstandingAlarmsClear;
      outstandingAlarmsClear << "Update platform.AlarmsOutstanding set isCleared=true where "
         << "OriginatedByNEID=" << alarmEventMessage->getSourceAddress().neid 
         << " and ApplicableToNEID=" << alarmEventMessage->getNEID() 
         << " and AlarmCode=" << alarmEventMessage->getAlarmCode()
         << " and ManagedObject=" << alarmEventMessage->getManagedObject()
         << " and ManagedObjectInstance=" << alarmEventMessage->getManagedObjectInstance()
         << " and Pid=" << alarmEventMessage->getPid() << ";" << ends;

      // Execute the clear against the database
      if (connectionHandle->executeCommand(outstandingAlarmsClear.str().c_str()) == ERROR)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "AlarmsOutstanding clear alarm execution failed",0,0,0,0,0,0);
         DataManager::releaseConnection(connectionHandle);
         return ERROR;
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Successfully cleared alarm in outstanding alarms list",0,0,0,0,0,0);
      }//end else
      // Release the connection
      DataManager::releaseConnection(connectionHandle);


      // Send the Clear Alarm to the Clients/NOCs

//TODO


   }//end if Clear Alarm
   // otherwise its an alarm to raise
   else if (alarmEventMessage->getMessageType() == ALARM_MESSAGE)
   {
      // Get access to a database connection
      DbConnectionHandle* connectionHandle = DataManager::reserveConnection(CLIENT_AGENT_CONNECTION);
      if (connectionHandle == NULL)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error reserving connection",0,0,0,0,0,0);
         return ERROR;
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Successfully reserved Connection for AlarmEvent Management",0,0,0,0,0,0);
      }//end else

      // Issue a debug log if the Alarm is being Raised on behalf (by proxy) of another node
      if (alarmEventMessage->getSourceAddress().neid != alarmEventMessage->getNEID())
      {
         ostringstream ostr;
         ostr << "Received Proxy Raise Alarm with id (" << alarmEventMessage->getAlarmCode()
              << ") raised on behalf of NEID (" << alarmEventMessage->getNEID()
              << ") by NEID (" << alarmEventMessage->getSourceAddress().neid << ")" << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
      }//end if
      else
      {
         ostringstream ostr;
         ostr << "Received Raise Alarm with id (" << alarmEventMessage->getAlarmCode()
              << ") from NEID (" << alarmEventMessage->getNEID() << ")" << ends;
         STRACELOG(DEBUGLOG, CLIENTAGENTLOG, ostr.str().c_str());
      }//end else

      // Massage the TimeStamp value to work in the database schema
      char timeBuffer[30];
      time_t timeStamp = alarmEventMessage->getTimestamp();
      strftime(timeBuffer, sizeof(timeBuffer), "%F %H:%M:%S.00%z", localtime(&timeStamp));
      // Truncate the time zone to 2 characters
      timeBuffer[strlen(timeBuffer) - 2] = '\0';

      ostringstream outstandingAlarmsInsert;
      // Build an insert statement to add the alarm record into the system-wide outstanding
      // alarms table. NOTE: Acknowledgement field is populated by the EMS operator, so not here
      outstandingAlarmsInsert << "Insert into platform.AlarmsOutstanding (OriginatedByNEID, ApplicableToNEID,"
         << " AlarmCode, ManagedObject, ManagedObjectInstance, Pid, TimeStamp, IsCleared) "
         << "values (" << alarmEventMessage->getSourceAddress().neid  
         << "," << alarmEventMessage->getNEID() << "," << alarmEventMessage->getAlarmCode()
         << "," << alarmEventMessage->getManagedObject() << "," << alarmEventMessage->getManagedObjectInstance()
         << "," << alarmEventMessage->getPid() << ",'" << timeBuffer << "',false);" << ends;

      // Execute the update/insert against the database
      if (connectionHandle->executeCommand(outstandingAlarmsInsert.str().c_str()) == ERROR)
      {
         TRACELOG(ERRORLOG, CLIENTAGENTLOG, "AlarmsOutstanding alarm insert execution failed",0,0,0,0,0,0);
         DataManager::releaseConnection(connectionHandle);
         return ERROR;
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Successfully raised alarm into outstanding alarms list",0,0,0,0,0,0);
      }//end else
      // Release the connection
      DataManager::releaseConnection(connectionHandle);

      // Send the Raised Alarm to the Clients/NOCs

//TODO


   }//end else if Alarm
   // otherwise, its an event report, so we need to write it to our file
   else if (alarmEventMessage->getMessageType() == EVENT_REPORT_MESSAGE)
   {
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "Received Event Report with id (%d)",
         alarmEventMessage->getEventCode(),0,0,0,0,0);

//TODO: Move all of this code (including alarms/clears) somewhere else. We need to add command line parameters to 
//        control how many event Report file are kept, the size of each one, and how many
//        event reports do we process in between checking the file size for rollover.

      // NOTE: One other option we could use is to send the event reports to the syslog
      //   facility and have the OS write them to their own dedicated file. This would be
      //   useful if someone wanted the Event Reports to be sent off-card automatically.
      // However, its less processing intensive for us to just write them to a file that
      //   automatically rolls over. Then we can setup an httpd instance to allow users to
      //   view the event reports with some sort of browser.

      // For now, we just duplicate the file rollover mechanism used in LogProcessor.cpp (for
      // when we are managing our own log files mode)

//TODO:
      // Right now, we just write the BARE MsgMgr message to the file, but it DOES NOT contain
      // the event report description,etc. Upon initialization, we need to read all of the
      // Event Report Inventory Database Table out and store it in memory (some map container
      // or something). Then we will use those fields for writing to the file.
      eventReportFileStream_ << alarmEventMessage->toString() << endl << endl << flush;

      // For every nth event that we write to the file, we check the size of the file
      // If it exceeds the specified limit, then we rollover to the next file (and rename the existing ones)
      // NOTE: To tune the performance, this file size checking interval (every 10 events) may need changing.
      if ((numberKeptEventFiles_ != 0) && (fileCheckCounter_ == 10))
      {
         off_t fileSize;
         struct stat fileStats;
         // Check the size of the file
         if (stat(eventReportFileName_.c_str(), &fileStats))
         {
            TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Error stat'ing event report file to get file size",0,0,0,0,0,0);
         }//end if
         fileSize = fileStats.st_size;

         // If we have exceeded the size, then rollover all of the files. NOTE: here, if the
         // file size option was specified, then we use that number; otherwise we use whatever
         // the default is as specified in the constructor initialization list.
         if (fileSize >= sizeKeptEventFiles_)
         {
            // Existing event file extensions get incremented, but not past the specified number
            // of 'kept' event files (after that they get deleted)
            ifstream eventFile;
            for (int i = (numberKeptEventFiles_ - 1); i > 0; i--)
            {
               // check to see if the file exists (perhaps we haven't been running long enough to
               // accumulate all of these files yet)
               ostringstream currentFileName;
               currentFileName << eventReportFileName_ << "." << i;
               // Test to see if the file exists
               if (!stat(currentFileName.str().c_str(), &fileStats))
               {
                  // So, the file exists. If its the last file in the 'kept' range, delete it
                  if (i == (numberKeptEventFiles_ - 1))
                  {
                     if (remove(currentFileName.str().c_str()) == ERROR)
                     {
                        TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Event report file remove failed",0,0,0,0,0,0);
                     }//end if
                  }//end if
                  // Increment the file name
                  else
                  {
                     // Incremented file name
                     ostringstream newFileName;
                     newFileName << eventReportFileName_ << "." << (i + 1);
                     // Rename the file
                     if (rename(currentFileName.str().c_str(), newFileName.str().c_str()) == ERROR)
                     {
                        TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Event report file rename failed",0,0,0,0,0,0);
                     }//end if
                  }//end else
               }//end if
            }//end for

            // If its the first/active event file, then close the active event file stream, move
            // it to a new filename (with .1 extension) and open a new file (this is the i==0 case above)
            // Flush and close
            eventReportFileStream_ << flush;
            eventReportFileStream_.close();
            // Build the new filename
            ostringstream newFileName;
            newFileName << eventReportFileName_ << ".1";
            // Rename the file
            if (rename(eventReportFileName_.c_str(), newFileName.str().c_str()) == ERROR)
            {
               TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Active event report file rename failed",0,0,0,0,0,0);
            }//end if
            // Open the new/clean file
            eventReportFileStream_.open (eventReportFileName_.c_str(), ios_base::out | ios_base::app);
            // Check the new file stream
            if (eventReportFileStream_.fail())
            {
               ostringstream ostr;
               ostr << " Failed to reopen file " << eventReportFileName_ << " for events following rollover" << ends;
               STRACELOG(ERRORLOG, CLIENTAGENTLOG, ostr.str().c_str());
            }//end if
         }//end if

         // Reset the counter
         fileCheckCounter_ = 0;
      }//end if
      else
      {
         // Increment the counter
         fileCheckCounter_++;
      }//end else


   }//end else if Event Report
   else
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Unknown AlarmEvent Message Type received (%d)",
         alarmEventMessage->getMessageType(),0,0,0,0,0);
      return ERROR;
   }//end else
   return OK;
}//end processAlarmEventMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Setup the Mailbox framework
// Design:
//-----------------------------------------------------------------------------
bool ClientAgent::setupMailbox(const MailboxAddress& distributedAddress)
{
   // Create the Local Mailbox
   clientAgentMailbox_ = DistributedMailbox::createMailbox(distributedAddress);
   if (!clientAgentMailbox_)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Unable to create ClientAgent mailbox",0,0,0,0,0,0);
      return false;
   }//end if

//TAKE OUT:
clientAgentMailbox_->setDebugValue(10);

   // Create the message handlers and put them in the list
   messageHandlerList_ = new MessageHandlerList();
   if (!messageHandlerList_)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Unable to create message handler list",0,0,0,0,0,0);
      return false;
   }//end if

   // Create handlers for each message expected

   // Handler for AlarmEvent Messages
   MessageHandler alarmEventMessageHandler = makeFunctor((MessageHandler*)0,
      *this, &ClientAgent::processAlarmEventMessage);

   // Add the message handlers to the message handler list to be used by the MsgMgr
   // framework. Here, we use the MessageId - Note that we do not attempt to process
   // anymore messages of this type in the context of this mailbox processor

   messageHandlerList_->add(FAULT_MANAGER_ALARM_EVENT_MSG_ID, alarmEventMessageHandler);

   // Register support for distributed messages so that the DistributedMessageFactory
   // knows how to recreate them in 'MessageBase' form when they are received.

   // For receiving AlarmEventMessage notifications
   MessageBootStrapMethod alarmEventMessageBootStrapMethod = makeFunctor( 
      (MessageBootStrapMethod*)0, AlarmEventMessage::deserialize);
   MessageFactory::registerSupport(FAULT_MANAGER_ALARM_EVENT_MSG_ID, alarmEventMessageBootStrapMethod);

   return true;
}//end setupMailbox


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: main function for ClientAgent binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
   bool localLoggerMode = false;

   // Turn of OS limits and enable core file generation
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // If this application does NOT need command line arguments, then uncomment (to
   // prevent compiler warning for unused variables):
   //int tmpInt __attribute__ ((unused)) = argc;
   //char** tmpChar __attribute__ ((unused)) = argv;

   // Create usage string
   ostringstream usageString;
   string str4spaces = "    ";
   usageString << "Usage:\n  ClientAgent\n" << str4spaces 
               /* command line options */
               << "-h Restrict the use of hostnames in IORs. Invokes -ORBDottedDecimalAddresses=1 on the Orb\n" << str4spaces 
               << "-d Enable debug messages from within the Orb\n" << str4spaces
               << "-v <Debug Level> Controls the level of debug messages from within the Orb (Max is 10)\n" << str4spaces
               << "-f <log filename> Redirect Orb logs to a file instead of the default stdout\n" << str4spaces
               << "-e <Endpoint> Direct the Orb to listen for connections on the interface specified by the endpoint URL\n" << str4spaces
               << "-i <Initial Reference> URL the Orb uses for initial resolution of the NameService. Invokes -ORBInitRef\n" << str4spaces
               << "-r <Default Init Ref> URL the Orb uses for default initial resolution of the NameService. Invokes -ORBDefaultInitRef\n"
               << str4spaces
               << "-l Local Logger Mode only (do not enqueue logs to shared memory)" << str4spaces;
   usageString << ends;

   // Validate that the user passed some arguments
   if (argc < 1)
   {
      cout << "Minimum argument requirements not met." << endl
           << usageString << endl;
      exit(ERROR);
   }//end if

   // Perform our own arguments parsing -before- we pass args to the class
   ACE_Get_Opt get_opt (argc, argv, "lhdv:f:e:i:r:");
   bool dottedDecimalIORFlag = false;
   bool debugFlag = false;
   string debugLevel = "0";
   string debugFilename;
   string endPointString;
   string initRef;
   string defaultInitRef;
   int c;
   while ((c = get_opt ()) != ERROR)
   {
      switch (c)
      {
         case 'l':
         {
            // Set local logger mode command line option flag
            localLoggerMode = true;
            break;
         }//end case
         case 'h':
         {
            // Set flag for enabling Dotted Decimal Addresses inside IORs 
            dottedDecimalIORFlag = true;
            break;
         }//end case
         case 'd':
         {
            // Set flag for enabling Debug logs within the Orb
            debugFlag = true;
            break;
         }//end case
         case 'v':
         {
            // Get the argument for setting the Debug Level within the Orb
            debugLevel = get_opt.optarg;
            break;
         }//end case
         case 'f':
         {
            // Get the argument for the file to redirect Orb Debug logs to
            debugFilename = get_opt.optarg;
            break; 
         }//end case
         case 'e':
         {
            // Get the argument for setting the Orb Listener Endpoint
            endPointString = get_opt.optarg;
            break;
         }//end case
         case 'i':
         {
            // Get the argument for the Initial Reference String url
            initRef = get_opt.optarg;
            break;
         }//end case
         case 'r':
         {
            // Get the argument for the Default Initial Reference String url
            defaultInitRef = get_opt.optarg;
            break;
         }//end case
         default:
         {
            cout << "Misunderstood option: " << c << endl;
            cout << usageString.str() << endl;
            exit(ERROR);
         }//end default
      }//end switch
   }//end while

   // Initialize the Logger with output to the Log Processor shared memory queue
   // unless the local logger mode command line option was specified (usually for debugging
   // and sending logs to local stdout)
   Logger::getInstance()->initialize(localLoggerMode);
   if (localLoggerMode)
   {
      // Turn on/off All relevant Subsystem logging levels
      Logger::setSubsystemLogLevel(OPMLOG, DEVELOPERLOG);
      Logger::setSubsystemLogLevel(CLIENTAGENTLOG, DEVELOPERLOG);
   }//end if

   // Initialize the OPM
   OPM::initialize();

   // Create the ClientAgent instance
   ClientAgent* clientAgent = new ClientAgent();
   if (!clientAgent)
   {
      TRACELOG(ERRORLOG, CLIENTAGENTLOG, "Could not create ClientAgent instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Initialize the ClientAgent instance
   if (clientAgent->initialize(dottedDecimalIORFlag, debugFlag, debugLevel, debugFilename, 
       endPointString, initRef, defaultInitRef) == ERROR)
   {
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "ClientAgent failed initialization",0,0,0,0,0,0);
      exit(ERROR);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, CLIENTAGENTLOG, "ClientAgent successfully started with PID=%d", ACE_OS::getpid(),0,0,0,0,0);
   }//end else

   // Start ClientAgent's mailbox processing loop to handle notifications. Here we block
   // and run forever until we are shutdown by the SIGINT/SIGQUIT signal
   clientAgent->processMailbox();

   TRACELOG(ERRORLOG, CLIENTAGENTLOG, "ClientAgent Main Method exiting",0,0,0,0,0,0);
}//end main
