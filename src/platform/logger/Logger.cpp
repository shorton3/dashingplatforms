/******************************************************************************
*
* File name:   Logger.cpp
* Subsystem:   Platform Services
* Description: Implements the Logger Server.
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
#include <iostream>

#include <ace/Process_Semaphore.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "Logger.h"
#include "LogMessage.h"
#include "LoggerCommon.h"
#include "LoggerSMConfig.h"
#include "LoggerSMQueue.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

unsigned char* Logger::logSubSystemLevel_ = new unsigned char[MAX_LOG_SUBSYSTEM];

unsigned short Logger::logSequenceId_ = 0;

// Singleton instance variable
Logger* Logger::loggerInstance_ = NULL;

ACE_Process_Semaphore* Logger::processSemaphore_ = NULL;

bool Logger::sendOutputToLocal_ = false;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
Logger::Logger()
{
   loggerSMQueue_ = new LoggerSMQueue(LOGSM_QUEUENAME, LOGSM_QUEUEMUTEXNAME);
   loggerSMConfig_ = new LoggerSMConfig(LOG_CONFIG_NAME, LOG_CONFIG_MUTEXNAME);

   // Create/map the process semaphore. Initialize it to be blocked
   processSemaphore_ = new ACE_Process_Semaphore(0, LOGQUEUE_SEMAPHORE_NAME);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
Logger::~Logger()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return a singleton instance
// Design:
//-----------------------------------------------------------------------------
Logger* Logger::getInstance()
{ 
   if (loggerInstance_ == NULL)
   { 
      loggerInstance_ = new Logger();
   }//end if
   return loggerInstance_;
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Set the Log Level for a particular subsystem
// Design:
//-----------------------------------------------------------------------------
LogEntrySeverityType Logger::getSubsystemLogLevel(LogEntrySubSystemType subsystem)
{
   // update/set the local logSubSystemLevel_ array if we are initialized to use
   // shared memory
   if (!sendOutputToLocal_)
   {
      logSubSystemLevel_[subsystem] = getInstance()->loggerSMConfig_->getLogLevel(subsystem);
   }//end if

   return (LogEntrySeverityType)logSubSystemLevel_[subsystem];
}//end getSubsystemLogLevel

//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Set the Log Level for a particular subsystem
// Design:     
//-----------------------------------------------------------------------------
void Logger::setSubsystemLogLevel(LogEntrySubSystemType subsystem, LogEntrySeverityType severityLevel)
{
   // Validate the range of the subsystem and severity level (remember ALL = 0)
   if( (subsystem >= 1) && (subsystem < MAX_LOG_SUBSYSTEM) && 
       (severityLevel > 0) && (severityLevel < 7) )
   {
      logSubSystemLevel_[subsystem] = severityLevel;
      // Set it in shared memory as well (note the PI position independent pointer
      // is already set in the LoggerSMConfigValues constructor) if we are initialized to
      // use shared memory
      if (!sendOutputToLocal_)
      {
         getInstance()->loggerSMConfig_->setLogLevel(subsystem, severityLevel);
      }//end if
   }//end if
   // If the subsystem is ALL, set all subsystem levels to the given level 
   else if( (subsystem == ALL_LOG_SUBSYSTEMS) && (severityLevel > 0) && (severityLevel < 7) )
   {
      for(int i = 1; i < MAX_LOG_SUBSYSTEM; i++)
      {
         logSubSystemLevel_[i] = severityLevel;
         // Set it in shared memory as well (note the PI position independent pointer
         // is already set in the LoggerSMConfigValues constructor) if we are initialized to
         // use shared memory
         if (!sendOutputToLocal_)
         {
            getInstance()->loggerSMConfig_->setLogLevel(i, severityLevel);
         }//end if
      }//end for
   }//end else if
}//end setSubsystemLogLevel


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Enqueue a log message for output
// Design:      Memory Management here: p_logMessage is assumed to be statically
//              allocated memory when this method is used (printf style formatting
//              arguments are passed in as well). For this reason, we only need
//              to make a copy of the buffer for shared memory allocation.
//              The 'logMessage' struct that we create on the first line
//              here will be deleted (including its logMessage buffer--deleted 
//              by its destructor) when this method goes out of scope.
//
//              A performance optimization for this logger should be to create pool of
//              logMessage structs on the heap and reuse them:  reserve one here and
//              fill its contents, then pass it into the log queue where it
//              gets deleted on the other side once it is output to where-ever.
//              NOTE: this pool should not use OPM (circular dependency).
//-----------------------------------------------------------------------------
#if __WORDSIZE == 64
void Logger::traceLog(LogEntrySubSystemType subsystem, LogEntrySeverityType severity, int pid, const char* sourceFile, int sourceLine, const char* p_logMessage, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
#else // WORDSIZE is 32bits
void Logger::traceLog(LogEntrySubSystemType subsystem, LogEntrySeverityType severity, int pid, const char* sourceFile, int sourceLine, const char* p_logMessage, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
#endif
{
   LogMessage logMessage;

   logMessage.isStringLog = false;
   logMessage.sequenceId = Logger::logSequenceId_++;
   logMessage.subsystem = subsystem;
   logMessage.severityLevel = severity;
   logMessage.pid = pid;
   logMessage.sourceLine = sourceLine;
   // put on the current timestamp
   logMessage.timeStamp = time(0);

   // Truncate this string if necessary
   ACE_OS::strncpy(logMessage.sourceFile, sourceFile, LOG_SOURCE_FILE_SIZE);
   logMessage.sourceFile[LOG_SOURCE_FILE_SIZE - 1] = '\0'; /* make sure to terminate */
   logMessage.sourceFilePI = logMessage.sourceFile;

   // Truncate this string if necessary
   ACE_OS::strncpy(logMessage.logMessage, p_logMessage, LOG_BUFFER_SIZE);
   logMessage.logMessage[LOG_BUFFER_SIZE - 1] = '\0'; /* make sure to terminate */
   logMessage.logMessagePI = logMessage.logMessage;

   logMessage.arg1 = arg1;
   logMessage.arg2 = arg2;
   logMessage.arg3 = arg3;
   logMessage.arg4 = arg4;
   logMessage.arg5 = arg5;
   logMessage.arg6 = arg6;

   if (!sendOutputToLocal_)
   {
      // Post the logMessage into the message queue
      if(loggerInstance_->getQueue()->enqueueLog(logMessage) == ERROR)
      {
         cout << "Logger: Error enqueuing log to shared memory" << endl;
      }//end if

      // Release the Blocking Process Semaphore to wake-up the dequeue thread and increment the semaphore
      processSemaphore_->release();
   }//end if
   else
   {
      // Send logMessage contents to stdout/stderr by formatting the log 
      // message here in this thread context  
      char* buffer = new char[LOG_BUFFER_SIZE];
      LoggerCommon::formatLogMessage(&logMessage, buffer);
      // Use a larger buffer for wrapped text (fudge factor here!)
      char* wrapBuffer = new char[LOG_BUFFER_SIZE + 200];
      char* outputBuffer = LoggerCommon::wrapFormattedLogText(buffer, wrapBuffer);
      // For now, output to stdout
      printf(outputBuffer);
      fflush(stdout);
      fflush(stderr);
      delete [] buffer;
      delete [] wrapBuffer;
   }//end else
}//end traceLog


//-----------------------------------------------------------------------------
// Method Type: STATIC 
// Description: Enqueue a string log message for output (make a copy before
//              inserting in the queue.
// Design:      Memory Management here: p_logMessage gets deleted when the calling
//              method goes out of scope since it was probably a stack variable
//              there. This is why we make a copy of p_logMessage before enqueuing
//              it to our Log Server. The 'logMessage' struct that we create on
//              the first line here will be deleted (including its logMessage
//              buffer--deleted by its destructor) when this method goes out of scope. 
//
//              A performance optimization for this logger should be to create pool of
//              logMessage structs on the heap and reuse them:  reserve one here and 
//              fill its contents, then pass it into the log queue where it 
//              gets deleted on the other side once it is output to where-ever.
//              NOTE: this pool should not use OPM (circular dependency).
//-----------------------------------------------------------------------------
void Logger::straceLog(LogEntrySubSystemType subsystem, LogEntrySeverityType severity, int pid, const char* sourceFile, int sourceLine, const char* p_logMessage)
{
   LogMessage logMessage;

   logMessage.isStringLog = true;
   logMessage.sequenceId = logSequenceId_++;
   logMessage.subsystem = subsystem;
   logMessage.severityLevel = severity;
   logMessage.pid = pid;
   logMessage.sourceLine = sourceLine;
   // Put on the current timestamp
   logMessage.timeStamp = time(0);

   // Truncate this string if necessary
   ACE_OS::strncpy(logMessage.sourceFile, sourceFile, LOG_SOURCE_FILE_SIZE);
   logMessage.sourceFile[LOG_SOURCE_FILE_SIZE - 1] = '\0'; /* make sure to terminate */
   logMessage.sourceFilePI = logMessage.sourceFile;
                                                                                                                                 
   // Truncate this string if necessary
   ACE_OS::strncpy(logMessage.logMessage, p_logMessage, LOG_BUFFER_SIZE);
   logMessage.logMessage[LOG_BUFFER_SIZE - 1] = '\0'; /* make sure to terminate */
   logMessage.logMessagePI = logMessage.logMessage;

   if (!sendOutputToLocal_)
   {
      // Post the logMessage into the message queue
      if(loggerInstance_->getQueue()->enqueueLog(logMessage) == ERROR)
      {
         cout << "Logger: Error enqueuing log to shared memory" << endl;
      }//end if

      // Release the Blocking Process Semaphore to wake-up the dequeue thread and increment the semaphore
      processSemaphore_->release();
   }//end if
   else
   {
      // Send logMessage contents to stdout/stderr by formatting the log
      // message here in this thread context
      char* buffer = new char[LOG_BUFFER_SIZE];
      LoggerCommon::formatLogMessage(&logMessage, buffer);
      // Use a larger buffer for wrapped text (fudge factor here!)
      char* wrapBuffer = new char[LOG_BUFFER_SIZE + 200];
      char* outputBuffer = LoggerCommon::wrapFormattedLogText(buffer, wrapBuffer);
      // For now, output to stdout
      printf(outputBuffer);
      fflush(stdout);
      fflush(stderr);
      delete [] buffer;
      delete [] wrapBuffer;
   }//end else
}//end straceLog


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform initialization of shared memory queue, etc.
// Design:
//-----------------------------------------------------------------------------
int Logger::initialize(bool sendOutputToLocal)
{
   sendOutputToLocal_ = sendOutputToLocal;
   // if we are NOT going to bypass shared memory and output to local stdout/stderr,
   // then we need to setup the Shared Memory Queue for the logger
   if (!sendOutputToLocal_)
   {
      if (loggerSMConfig_->initialize() == ERROR)
      {
         cout << "Logger: Unable to setup Logger config" << endl;
         return ERROR;
      }//end if
      if (loggerSMQueue_->setupQueue() == ERROR)
      {
         cout << "Logger: Unable to setup Logger queue" << endl;
         return ERROR;
      }//end if
   }//end if
   else
   {
      // If we are bypassing shared memory, then we need to initialize the local subsystem log levels array
      for (int subsystem = ALL_LOG_SUBSYSTEMS; subsystem < MAX_LOG_SUBSYSTEM; subsystem++)
      {
         logSubSystemLevel_[subsystem] = DEVELOPERLOG;
      }//end for
   }//end else
   
   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return a pointer to the Shared Memory queue                                        
// Design: 
//-----------------------------------------------------------------------------
LoggerSMQueue* Logger::getQueue()
{
   return loggerSMQueue_;
}//end getQueue


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

