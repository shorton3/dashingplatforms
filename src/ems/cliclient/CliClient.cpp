/******************************************************************************
*
* File name:   CliClient.cpp
* Subsystem:   EMS
* Description: Provides a CLI Client interface to the EMS' Client Agent
*              through CORBA.
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
#include <sstream>

#include <ace/Get_Opt.h>
#include <ace/Reactor.h>
#include <ace/Signal.h>
#include <ace/Sig_Adapter.h>

#include <tao/ORB_Core.h>
#include <orbsvcs/CosNamingC.h>
#include <tao/Utils/ORB_Manager.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliClient.h"

#include "platform/logger/Logger.h"

#include "platform/threadmgr/ThreadManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)CLI Client"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
CliClient::CliClient()
          : sessionImplPtr_(NULL)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
CliClient::~CliClient()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Gracefully shutdown the Application and all of its child threads
// Design:
//-----------------------------------------------------------------------------
void CliClient::shutdown()
{

}//end shutdown


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialization method
// Design:
//-----------------------------------------------------------------------------
int CliClient::initialize(bool dottedDecimalIORFlag, bool debugFlag, string& debugLevel,
     string& debugFilename, string& endPointString, string& initRef, string& defaultInitRef)
{
   char* exceptionMessage = NULL;

   try
   {
      exceptionMessage = "While ORB Manager init";

      // Build the Orb.init arguments. This is necessary since there is some bug that prevents
      // the Orb from initializing properly after the arguments have been run through ACE::Get_Opt.
      int numbArgs = 0;
      char** orbArgs = new char *[20];
      orbArgs[numbArgs++] = "CliClient";
      orbArgs[numbArgs++] = "-ORBDottedDecimalAddresses";
      if (dottedDecimalIORFlag == true)
      {
         TRACELOG(DEBUGLOG, CLICLIENTLOG, "Restricting IORs to use IP Addresses only",0,0,0,0,0,0);
         orbArgs[numbArgs++] = "1";
      }//end if
      else
      {
         TRACELOG(DEBUGLOG, CLICLIENTLOG, "IORs allowed to contain Hostnames for DNS resolution",0,0,0,0,0,0);
         orbArgs[numbArgs++] = "0";
      }//end else
      if (endPointString.empty() == false)
      {
         TRACELOG(DEBUGLOG, CLICLIENTLOG, "Orb instructed to use endpoints: %s",endPointString.c_str(),0,0,0,0,0);
         orbArgs[numbArgs++] = "-ORBListenEndpoints";
         orbArgs[numbArgs++] = (char*)endPointString.c_str();
      }//end if
      if (initRef.empty() == false)
      {
         TRACELOG(DEBUGLOG, CLICLIENTLOG, "Orb instructed to use initial reference: %s", initRef.c_str(),0,0,0,0,0);
         orbArgs[numbArgs++] = "-ORBInitRef";
         orbArgs[numbArgs++] = (char*)initRef.c_str();
      }//end if
      if (defaultInitRef.empty() == false)
      {
         TRACELOG(DEBUGLOG, CLICLIENTLOG, "Orb instructed to use default initial reference: %s",
            defaultInitRef.c_str(),0,0,0,0,0);
         orbArgs[numbArgs++] = "-ORBDefaultInitRef";
         orbArgs[numbArgs++] = (char*)defaultInitRef.c_str();
      }//end if
      if (debugFlag == true)
      {
         TRACELOG(DEBUGLOG, CLICLIENTLOG, "Orb internal debug logs enabled to level %s",
            debugLevel.c_str(),0,0,0,0,0);
         orbArgs[numbArgs++] = "-ORBDebug";
         orbArgs[numbArgs++] = "-ORBDebugLevel";
         orbArgs[numbArgs++] = (char*)debugLevel.c_str();
         if (debugFilename.empty() == false)
         {
            TRACELOG(DEBUGLOG, CLICLIENTLOG, "Orb internal debug logs redirected to file %s",
               debugFilename.c_str(),0,0,0,0,0);
            orbArgs[numbArgs++] = "-ORBLogFile";
            orbArgs[numbArgs++] = (char*)debugFilename.c_str();
         }//end if
      }//end if

      TRACELOG(DEBUGLOG, CLICLIENTLOG, "Calling orb init with %d arguments",numbArgs,0,0,0,0,0);

      // Initialize the Orb Manager - for GIOP 1.1, try: -ORBEndPoint iiop://1.1@<host>:<port>
      if (orbManager_.init (numbArgs, orbArgs) == ERROR)
      {
         TRACELOG(ERRORLOG, CLICLIENTLOG, "Error initializing the orb manager",0,0,0,0,0,0);
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
      exceptionMessage = "While resolving agent objects from the Naming Service";
      resolveAgentObjects();   
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
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
   // management. Here, returns the OS assigned unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)CliClient::runOrb, (void*) 0, "CliClientOrb", true, THR_NEW_LWP|THR_DETACHED);

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Handle Menu Option user input
// Design:
//-----------------------------------------------------------------------------
void CliClient::processMenu()
{
   // Pass in reference to all of the interface objects FOR NOW. This so that
   // that the Menu Handlers can invoke the remote methods. This probably needs
   // to be changed so that these are given by static methods called on this
   // class -- because the references may be regenerated/replaced if the connection
   // becomes stale and objects need to be re-resolved from the Naming Service!!!

   // This call also starts the menu display loop
   TRACELOG(DEBUGLOG, CLICLIENTLOG, "Initializing Menu Controller",0,0,0,0,0,0);
   cliMenuController_.initialize(session_I_ptr_, platformConfig_I_ptr_, alarmInterface_ptr_);

}//end processMenu


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string CliClient::toString()
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
// Description: Create the IDL-specific object interfaces
// Design:
//-----------------------------------------------------------------------------
int CliClient::createIDLObjects(PortableServer::POA_var& rootPOA)
{
   char* exceptionMessage = NULL;
   try
   {
      // Create an instance of the Session interface object. This object is
      // responsible for monitoring of agent/client communication
      ACE_NEW_RETURN (sessionImplPtr_, session_Session_I_i(), 0);

      PortableServer::ObjectId_var sessionOid = PortableServer::string_to_ObjectId ("CLISession");

      exceptionMessage = "While activating CLI Client Session";
      rootPOA->activate_object_with_id (sessionOid.in(), sessionImplPtr_);

      // For debugging, here we get an Object reference for session object.
      // This verifies that the ORB is correctly managing the object.
      exceptionMessage = "While Session::_this";
      session::Session_I* sessionVar = sessionImplPtr_->_this();

      // Stringify the object reference and print it out
      exceptionMessage = "While object_to_string";
      CORBA::String_var sessionIOR = this->orbManager_.orb()->object_to_string (sessionVar);

      // Print the IOR.
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "CLI Client Session IOR is <%s>", sessionIOR.in(),0,0,0,0,0);

      // Create additional IDL interface objects here...

//TODO: Need to implement a new part of the Alarms interface (register here) in order for us
// to receive Alarm notifications (async)


   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
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
int CliClient::initNamingService()
{
   const char *exceptionMessage = NULL;
   try
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "Attempting to resolve the Naming Service",0,0,0,0,0,0);

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
         TRACELOG(ERRORLOG, CLICLIENTLOG, "Unable to resolve Naming Service",0,0,0,0,0,0);
         // Need to implement retry semantics here!!


         return ERROR;
      }//end if

      exceptionMessage = "While narrowing Naming Service context";
      namingContextVar_ = CosNaming::NamingContext::_narrow (namingObj.in());

      // Check to see if there was a problem performing narrow
      if (CORBA::is_nil (namingContextVar_.in()))
      {
         TRACELOG(ERRORLOG, CLICLIENTLOG, "Naming Service Context reference is null",0,0,0,0,0,0);
         // Implement retry semantics here (if necessary)!!

         return ERROR;
      }//end if

      // Bind the Session IDL object into the naming service. This is the top level
      // interface from which all other interfaces should be retrieved. So, we should not
      // need to bind other interface objects to the Naming Service later.
      CosNaming::Name sessionName (1);
      sessionName.length (1);
      // TBD: This should be replaced with a unique identifier that distinguishes this
      // CLI Client from other possible CLI client. Needs discussion.
      sessionName[0].id = CORBA::string_dup ("ClientSession");
      sessionName[0].kind = CORBA::string_dup ("client");
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "CLI Client Naming Service Bind Name is ClientSession", 0,0,0,0,0,0);

      exceptionMessage = "While using Session _this";
      session::Session_I* sessionVar = sessionImplPtr_->_this();

      exceptionMessage = "While rebinding Session";
      namingContextVar_->rebind (sessionName, sessionVar);
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
      return ERROR;
   }//end catch
   return OK;
}//end initNamingService


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Resolve/retrieve our Agent's top level interface objects from
//   the Naming Service
// Design:
//-----------------------------------------------------------------------------
int CliClient::resolveAgentObjects()
{
   const char* exceptionMessage = NULL;
   try
   {
      // Resolve the Agent's Session reference which is the top level object reference. From
      // there we can get references to all of the other remote interfaces
      CosNaming::Name agentSessionContextName (1);
      agentSessionContextName.length (1);
      // NOTE, TBD: This needs to be replaced with a unique distinguishing string for
      // each Agent (redundant). For now, we only support 1 agent!!!!
      agentSessionContextName[0].id = CORBA::string_dup ("AgentSession");
      agentSessionContextName[0].kind = CORBA::string_dup ("server");

      if (CORBA::is_nil (namingContextVar_.in()))
      {
         TRACELOG(ERRORLOG, CLICLIENTLOG, "Naming Service Context reference is NULL",0,0,0,0,0,0);
         return ERROR;
      }//end if

      TRACELOG(DEBUGLOG, CLICLIENTLOG, "Resolving Agent's Session Interfaces",0,0,0,0,0,0);
      exceptionMessage = "While resolving Agent's session interfaces";
      CORBA::Object_var agentSessionVar = namingContextVar_->resolve(agentSessionContextName);

      exceptionMessage = "While narrowing the Agent's Session Interface";
      session_I_ptr_ = session::Session_I::_narrow (agentSessionVar.in());

      if (CORBA::is_nil (session_I_ptr_) || session_I_ptr_->_non_existent())
      {
         TRACELOG(ERRORLOG, CLICLIENTLOG, "Failed to resolve Agent's Session Interfaces in Naming Service",0,0,0,0,0,0);
         return ERROR;
      }//end if

      // Display list of Clients and their IORs
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "IOR of the Agent Session object is %s",
         this->orbManager_.orb()->object_to_string(session_I_ptr_),0,0,0,0,0);

      // Set the reference of the Agent Session object into our own Session object 
      // as the 'Associated Session'
      sessionImplPtr_->setAssociatedSession(session_I_ptr_);

      // Resolve the Agent's PlatformConfig reference for now. Later, we may need to use
      // an interface method to retrieve this object from the session top-level object
      CosNaming::Name agentPlatformConfigContextName(1);
      agentPlatformConfigContextName.length(1);

      agentPlatformConfigContextName[0].id = CORBA::string_dup("PlatformConfig");
      agentPlatformConfigContextName[0].kind = CORBA::string_dup("server");

      TRACELOG(DEBUGLOG, CLICLIENTLOG, "Resolving Agent's Platform Config Interface",0,0,0,0,0,0);
      exceptionMessage = "While resolving Agent's Platform Config interface";
      CORBA::Object_var agentPlatformConfigVar = namingContextVar_->resolve(agentPlatformConfigContextName);

      exceptionMessage = "While narrowing the Agent's Platform Config Interface";
      platformConfig_I_ptr_ = platformConfig::platformConfig_I::_narrow(agentPlatformConfigVar.in());

      if (CORBA::is_nil (platformConfig_I_ptr_) || (platformConfig_I_ptr_->_non_existent()))
      {
         TRACELOG(ERRORLOG, CLICLIENTLOG, "Failed to resolve Agent's Platform Config Interface in Naming Service",0,0,0,0,0,0);
         return ERROR;
      }//end if

      // Display the IOR
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "IOR of the Agent Platform Config object is %s",
         this->orbManager_.orb()->object_to_string(platformConfig_I_ptr_),0,0,0,0,0);

      // Resolve the Agent's Alarm reference for now. Later, we may need to use
      // an interface method to retrieve this object from the session top-level object
      CosNaming::Name agentAlarmContextName(1);
      agentAlarmContextName.length(1);

      agentAlarmContextName[0].id = CORBA::string_dup("Alarms");
      agentAlarmContextName[0].kind = CORBA::string_dup("server");

      TRACELOG(DEBUGLOG, CLICLIENTLOG, "Resolving Agent's Alarms Interface",0,0,0,0,0,0);
      exceptionMessage = "While resolving Agent's Alarms interface";
      CORBA::Object_var alarmInterface_var = namingContextVar_->resolve(agentAlarmContextName);

      exceptionMessage = "While narrowing the Agent's Alarms Interface";
      alarmInterface_ptr_ = alarms::alarmInterface::_narrow(alarmInterface_var.in());

      if (CORBA::is_nil (alarmInterface_ptr_) || (alarmInterface_ptr_->_non_existent()))
      {
         TRACELOG(ERRORLOG, CLICLIENTLOG, "Failed to resolve Agent's Alarms Interface in Naming Service",0,0,0,0,0,0);
         return ERROR;
      }//end if

      // Display the IOR
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "IOR of the Agent Alarms object is %s",
         this->orbManager_.orb()->object_to_string(alarmInterface_ptr_),0,0,0,0,0);

   }//end try
   //catch NamingContext error
   catch (CosNaming::NamingContext::NotFound notFoundException)
   {
      TRACELOG(ERRORLOG, CLICLIENTLOG, "CosNaming Naming Service Context not found %s: %s", 
         exceptionMessage, notFoundException._info().c_str(),0,0,0,0);
      return ERROR;
   }//end catch
   catch (CosNaming::NamingContext::CannotProceed cannotProceed)
   {
      TRACELOG(ERRORLOG, CLICLIENTLOG, "CosNaming Cannot proceed user exception %s: %s",
         exceptionMessage, cannotProceed._info().c_str(),0,0,0,0);
      return ERROR;
   }//end catch
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
      return ERROR;
   }//end catch
   return OK;
}//end resolveAgentObjects


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method used to start the TAO Orb in its own dedicated thread
// Design:
//-----------------------------------------------------------------------------
void CliClient::runOrb()
{
   // Start the orb main event loop - THIS IS AN INFINITE BLOCKING CALL
   TAO_ORB_Core_instance()->orb()->run();
}//end runOrb


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Type: main function for CliClient binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
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
   usageString << "Usage:\n  CliClient\n" << str4spaces
               /* command line options */
               << "-h Restrict the use of hostnames in IORs. Invokes -ORBDottedDecimalAddresses=1 on the Orb\n" << str4spaces
               << "-d Enable debug messages from within the Orb\n" << str4spaces
               << "-l <Debug Level> Controls the level of debug messages from within the Orb (Max is 10)\n" << str4spaces
               << "-f <log filename> Redirect Orb logs to a file instead of the default stdout\n" << str4spaces
               << "-e <Endpoint> Direct the Orb to listen for connections on the interface specified by the endpoint URL\n" << str4spaces
               << "-i <Initial Reference> URL the Orb uses for initial resolution of the NameService. Invokes -ORBInitRef\n" << str4spaces
               << "-r <Default Init Ref> URL the Orb uses for default initial resolution of the NameService. Invokes -ORBDefaultInitRef\n"
               << str4spaces;
   usageString << ends;

   // Validate that the user passed some arguments
   if (argc < 1)
   {
      cout << "Minimum argument requirements not met." << endl
           << usageString << endl;
      exit(ERROR);
   }//end if

   // Perform our own arguments parsing -before- we pass args to the class
   ACE_Get_Opt get_opt (argc, argv, "hdl:f:e:i:r:");
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
         case 'l':
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

   // Initialize the Logger with local-only output since we probably don't want to log 
   // these logs for the CLI through the shared memory queue (MAYBE ??)
   Logger::getInstance()->initialize(true);

   // Turn on All relevant Subsystem logging levels -- NOTE, for CliClient, this line
   // should match whatever the default datafill is in the platform.LogLevels table. We
   // need this to be true so that the user experience is consistent
   Logger::setSubsystemLogLevel(CLICLIENTLOG, DEVELOPERLOG);

   // Create an adapter to shutdown this application 
   ACE_Sig_Adapter signalAdapter ((ACE_Sig_Handler_Ex) CliClient::shutdown);
   
   // Specify which types of signals we want to trap
   ACE_Sig_Set signalSet;
   signalSet.sig_add (SIGINT);
   signalSet.sig_add (SIGQUIT);

   // Register ourselves to receive signals so we can shut down gracefully.
   if (ACE_Reactor::instance()->register_handler (signalSet, &signalAdapter) == ERROR)
   {
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Error registering for OS signals",0,0,0,0,0,0);
   }//end if

   // Initialize the OPM
   //OPM::initialize();

   // Create the CliClient instance
   CliClient* cliClient = new CliClient();
   if (!cliClient)
   {
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Could not create CliClient instance",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Initialize the CliClient instance
   if (cliClient->initialize(dottedDecimalIORFlag, debugFlag, debugLevel, debugFilename,
       endPointString, initRef, defaultInitRef) == ERROR)
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "CliClient failed initialization",0,0,0,0,0,0);
      exit(ERROR);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "CliClient successfully started with PID=%d", ACE_OS::getpid(),0,0,0,0,0);
   }//end else

   // Start CliClient's menu processing loop to handle user input. Here we block
   // and run forever until we are shutdown by the SIGINT/SIGQUIT signal
   cliClient->processMenu();
}//end main
