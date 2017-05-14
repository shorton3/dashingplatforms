/******************************************************************************
*
* File name:   CliMenuPlatformConfig.h
* Subsystem:   EMS
* Description: Static handler methods for platform configuration requests.
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

#include <tao/ORB_Core.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliMenuPlatformConfig.h"

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
CliMenuPlatformConfig::CliMenuPlatformConfig() : errorString_("DefaultError")
{
}//end Constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
CliMenuPlatformConfig::~CliMenuPlatformConfig()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize static variables to be used by the methods
// Design:     
//-----------------------------------------------------------------------------
void CliMenuPlatformConfig::initialize(platformConfig::platformConfig_I_ptr platformConfig_I_ptr)
{
   platformConfig_I_ptr_ = platformConfig_I_ptr;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve all of the subsystem / severity log level pairs from the agent
// Design:
//-----------------------------------------------------------------------------
string CliMenuPlatformConfig::getLogLevels()
{
   char* exceptionMessage = NULL;

   if (CORBA::is_nil (platformConfig_I_ptr_) || platformConfig_I_ptr_->_non_existent())
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "platformConfigIPtr is NULL",0,0,0,0,0,0);
      return errorString_;
   }//end if

   ostringstream ostr;
   try
   {
      exceptionMessage = "While retrieving log levels";

      platformConfig::platformConfigLogLevels_var logLevels_var;

      platformConfig_I_ptr_->getLogLevels(logLevels_var.out());

      if (logLevels_var.in().length() != MAX_LOG_SUBSYSTEM)
      {
         TRACELOG(WARNINGLOG, CLICLIENTLOG, "Retrieved Log Levels have too few elements: %d, should be %d",
            logLevels_var.in().length(),MAX_LOG_SUBSYSTEM,0,0,0,0);
         return errorString_;
      }//end if
      // Populate the Log Levels
      ostr <<    "  Subsystem            LogLevel   " << endl;
      ostr <<    "--------------------   -------------------" << endl;
      for (unsigned int i = 0; i < logLevels_var.in().length(); i++)
      {
         string name = logLevels_var[CORBA::ULong(i)].subsystemName.in();
         // Justify the fields with padding
         name.resize(15, ' ');
         int id = logLevels_var[CORBA::ULong(i)].subsystemEnum;
         int severity = logLevels_var[CORBA::ULong(i)].severityLevel;

         ostr << "  " << name << "(" << id << ")" << "   ";
         switch (severity)
         {
            case ERRORLOG:
               ostr << "ERROR LOG ";
               break;
            case WARNINGLOG:
               ostr << "WARNING LOG ";
               break;
            case INFOLOG:
               ostr << "INFO LOG";
               break;
            case DEBUGLOG:
               ostr << "DEBUG LOG";
               break;
            case DEVELOPERLOG:
               ostr << "DEVELOPER LOG";
               break;
            default:
               ostr << "UNKNOWN LEVEL(" << severity << ")";
               break;
         }//end switch
         ostr << endl;

         // For CLI Client Subsystem, set our local log levels to match what is in the database.
         // This may be confusing to the user, so may need to be later disabled.
         if (i == CLICLIENTLOG)
         {
            Logger::setSubsystemLogLevel((LogSubSystemType)i, (LogSeverityType)severity);
         }//end if
      }//end for
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
   }//end catch
   return ostr.str();
}//end getLogLevels


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the log level severity based on the subsystem id
// Design:
//-----------------------------------------------------------------------------
void CliMenuPlatformConfig::setLogLevel(int subsystem, int severityLevel)
{
   char* exceptionMessage = NULL;

   if (CORBA::is_nil (platformConfig_I_ptr_) || platformConfig_I_ptr_->_non_existent())
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "platformConfigIPtr is NULL",0,0,0,0,0,0);
      return;
   }//end if

   try
   {
      exceptionMessage = "While setting Log Level";

      // Allocate the logLevel structure
      platformConfig::platformConfigLogLevel* logLevel;
      ACE_NEW(logLevel, platformConfig::platformConfigLogLevel);
  
      // Populate the structure
      logLevel->subsystemEnum = subsystem;
      logLevel->subsystemName = "";
      logLevel->severityLevel = severityLevel;

      // Call the method and pass in the structure
      platformConfig_I_ptr_->setLogLevel(*logLevel);

      // Change the subsystem log level locally in the Logger as well
      Logger::setSubsystemLogLevel((LogSubSystemType)subsystem, (LogSeverityType)severityLevel);
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
   }//end catch/end sendLock
}//end setLogLevel


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

