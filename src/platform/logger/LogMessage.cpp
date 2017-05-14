/******************************************************************************
*
* File name:   LogMessage.cpp
* Subsystem:   Platform Services
* Description: Log Message class encapsulates the information for a single
*              log entry so that it can be enqueued by the originating application
*              and then dequeued by the Log Processor process for transfer to the
*              selected output media (logfile, syslog, etc.)
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

#include <cstdio>
#include <cstring>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LogMessage.h"

#include "platform/common/Defines.h"

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
LogMessage::LogMessage()
           : isStringLog(false),
             sequenceId(0),
             subsystem(MAX_LOG_SUBSYSTEM),
             severityLevel(LAST_LOG_SEVERITY),
             pid(0),
             sourceLine(0),
             timeStamp(0),
             arg1(0),
             arg2(0),
             arg3(0),
             arg4(0),
             arg5(0),
             arg6(0)
{
   sourceFile[0] = '\0';
   sourceFilePI = sourceFile;
   logMessage[0] = '\0';
   logMessagePI = logMessage;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Copy Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
LogMessage::LogMessage( const LogMessage& rhs )
{
   isStringLog = rhs.isStringLog;
   sequenceId = rhs.sequenceId;
   subsystem = rhs.subsystem;
   severityLevel = rhs.severityLevel;
   pid = rhs.pid;
   sourceLine = rhs.sourceLine;
   timeStamp = rhs.timeStamp;
   
   ACE_OS::strcpy(sourceFile, rhs.sourceFile);
   sourceFilePI = sourceFile;

   ACE_OS::strcpy(logMessage, rhs.logMessage);
   logMessagePI = logMessage;

   arg1 = rhs.arg1;
   arg2 = rhs.arg2;
   arg3 = rhs.arg3;
   arg4 = rhs.arg4;
   arg5 = rhs.arg5;
   arg6 = rhs.arg6;
}//end Copy Constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LogMessage::~LogMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reset the data members
// Design:
//-----------------------------------------------------------------------------
void LogMessage::reset()
{
   isStringLog = false;
   sequenceId = 0;
   subsystem = MAX_LOG_SUBSYSTEM;
   severityLevel = LAST_LOG_SEVERITY;
   pid = 0;
   sourceLine = 0;
   timeStamp = 0;
   sourceFile[0] = '\0';
   sourceFilePI = sourceFile;
   logMessage[0] = '\0';
   logMessagePI = logMessage;
   arg1 = 0;
   arg2 = 0;
   arg3 = 0;
   arg4 = 0;
   arg5 = 0;
   arg6 = 0;
}//end reset


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded assignment operator
// Design:
//-----------------------------------------------------------------------------
LogMessage& LogMessage::operator= ( const LogMessage& rhs )
{
   if (this != &rhs)
   {
      isStringLog = rhs.isStringLog;
      sequenceId = rhs.sequenceId;
      subsystem = rhs.subsystem;
      severityLevel = rhs.severityLevel;
      pid = rhs.pid;
      sourceLine = rhs.sourceLine;
      timeStamp = rhs.timeStamp;

      ACE_OS::strcpy(sourceFile, rhs.sourceFile);
      sourceFilePI = sourceFile;

      ACE_OS::strcpy(logMessage, rhs.logMessage);
      logMessagePI = logMessage;

      arg1 = rhs.arg1;
      arg2 = rhs.arg2;
      arg3 = rhs.arg3;
      arg4 = rhs.arg4;
      arg5 = rhs.arg5;
      arg6 = rhs.arg6;
   }//end if
   return *this;
}//end assignment operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Equality Operator
// Design:
//-----------------------------------------------------------------------------
bool LogMessage::operator== (const LogMessage& rhs)
{
   if ( (isStringLog == rhs.isStringLog) &&
        (sequenceId == rhs.sequenceId) &&
        (subsystem == rhs.subsystem) &&
        (severityLevel == rhs.severityLevel) &&
        (pid == rhs.pid) &&
        (sourceLine == rhs.sourceLine) &&
        (timeStamp == rhs.timeStamp) &&
        (sourceFilePI == rhs.sourceFilePI) &&
        (logMessagePI == rhs.logMessagePI) &&
        (arg1 == rhs.arg1) &&
        (arg2 == rhs.arg2) &&
        (arg3 == rhs.arg3) &&
        (arg4 == rhs.arg4) &&
        (arg5 == rhs.arg5) &&
        (arg6 == rhs.arg6) )
   {
      return true;
   }//end if
   else
   {
      return false;
   }//end else
}//end equality operator


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

