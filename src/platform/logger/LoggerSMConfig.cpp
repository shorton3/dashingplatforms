/******************************************************************************
*
* File name:   LoggerSMConfig.cpp
* Subsystem:   Platform Services
* Description: This class handles storing and retrieving the Logger configuration
*              parameters (such as log level) to and from Shared Memory.
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

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LoggerSMConfig.h"

#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Based_Pointer_Basic<int>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Based_Pointer_Basic<int>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LoggerSMConfig::LoggerSMConfig(const char* configName, const char* coordinatingMutexName) 
             : shmemAllocator_(NULL),
               coordinatingMutex_(coordinatingMutexName),
               configName_(configName)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LoggerSMConfig::~LoggerSMConfig()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform initialization of the shared memory
// Design:
//-----------------------------------------------------------------------------
int LoggerSMConfig::initialize()
{
   //cout << "LoggerSMConfig initialize" << endl;

   void* loggerSMConfigValues = 0;

   // First let's get a pointer to the common shared memory allocator
   shmemAllocator_ = SharedMemoryManager::getAllocator();
   if (shmemAllocator_ == NULL)
   {
      return ERROR;
   }//end if

   // This is the easy case since if we find the structure name in the
   // memory-mapped file we know it's already initialized.
   if (shmemAllocator_->find (configName_.c_str(), loggerSMConfigValues) == 0)
   {
      cout << "Logger SM Config: found already existing SM Values structure" << endl;
      loggerSMConfigValues_ = (LoggerSMConfigValues*) loggerSMConfigValues;
      return OK;
   }//end if

   // Create a new values structure in shared memory (because we've just created a new memory-mapped file).
   loggerSMConfigValues = shmemAllocator_->malloc (sizeof (LoggerSMConfigValues));

   // If allocation failed ...
   if (loggerSMConfigValues == 0)
   {
      cout << "Logger SM Config: allocation for the memory size failed. Run as 'root' in order to allocate shared memory" << endl;
      return ERROR;
   }//end if

   // Instantiate/place the structure into the memory space
   new (loggerSMConfigValues) LoggerSMConfigValues ();
   if (shmemAllocator_->bind (configName_.c_str(), loggerSMConfigValues) == ERROR)
   {
      cout << "Logger SM Config: bind to the config values structure failed" << endl;
      shmemAllocator_->remove();
      return ERROR;
   }//end if

   // Keep a pointer to the config values structure in shared memory. This is what we will use to
   // read and write the values
   loggerSMConfigValues_ = (LoggerSMConfigValues*) loggerSMConfigValues;

   // Call reset on the config values structure in order to setup the PI (position
   // independent) pointer
   loggerSMConfigValues_->reset();

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Get the Log Level configuration values
// Design:
//-----------------------------------------------------------------------------
int LoggerSMConfig::getLogLevel(int subsystem)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);

   //cout << "LoggerSMConfig getLogLevel for subsystem(" << subsystem << ")" << endl;

   if (loggerSMConfigValues_ == NULL)
   {
      cout << "LoggerSMConfig values structure is NULL" << endl;
      return ERROR;
   }//end if

   // Use the PI object to access the config values
   //cout << "Log Level for subsystem(" << subsystem << ") is (" 
   //     << loggerSMConfigValues_->logLevelsPI[subsystem] << ")" << endl;

   return loggerSMConfigValues_->logLevelsPI[subsystem];
}//end getLogLevel


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the Log Level configuration values
// Design:
//-----------------------------------------------------------------------------
int LoggerSMConfig::setLogLevel(int subsystem, int severityLevel)
{
   ACE_GUARD_RETURN (ACE_Process_Mutex, ace_mon, coordinatingMutex_, ERROR);

   //cout << "LoggerSMConfig setLogLevel" << endl;

   if (loggerSMConfigValues_ == NULL)
   {
      cout << "LoggerSMConfig values structure is NULL" << endl;
      return ERROR;
   }//end if

   // No need to set the PI (Position Independent) pointer again (already been set),
   // (NOT SURE ABOUT THAT, so let's do it anyway!!)
   loggerSMConfigValues_->logLevels[subsystem] = severityLevel;
   loggerSMConfigValues_->logLevelsPI = loggerSMConfigValues_->logLevels;
   return OK;
}//end setLogLevel


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reset the contents of the shared memory configuration structure to defaults
// Design:
//-----------------------------------------------------------------------------
void LoggerSMConfig::reset()
{
   ACE_GUARD (ACE_Process_Mutex, ace_mon, coordinatingMutex_);

   //cout << "LoggerSMConfig reset" << endl;
   loggerSMConfigValues_->reset();
}//end reset


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string LoggerSMConfig::toString()
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

