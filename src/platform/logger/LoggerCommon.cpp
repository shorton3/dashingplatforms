/******************************************************************************
*
* File name:   LoggerCommon.cpp
* Subsystem:   Platform Services
* Description: Common Logging routines used by both the Logger.h client side
*              and the LogProcessor server side.
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
#include <time.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LoggerCommon.h"
#include "LogMessage.h"
#include "Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


// String form of the subsystem names
const char* logSubSystemName[] = {  "ALL SUBSYSTEMS",
                              /* Platform Subsystems */
                              "OPM",
                              "MSGMGR",
                              "PROCMGRLOG",
                              "RSRCMONLOG",
                              "DATAMGRLOG",
                              "FAULTMGRLOG",
                              "THREADLOG",

                              /* EMS Subsystems */
                              "CLIENTAGENTLOG",
                              "CLICLIENTLOG",

                              /* Application Subsystems */

                              "MAX SUBSYSTEM" };

// String form of the severity level names
const char* severityLevelName[] = { "FIRST LEVEL",
                              "ERROR",
                              "WARNING",
                              "INFO",
                              "DEBUG",
                              "DEVELOPER",
                              "LAST LEVEL" };

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LoggerCommon::~LoggerCommon()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Convert raw time stamp to readable time string
// Design:
//-----------------------------------------------------------------------------
void LoggerCommon::convertTimeToString(long time, char* buf)
{
   struct tm tNow;
   struct tm *t;

   t = localtime((time_t*)&time);

   memcpy(&tNow, t, sizeof(struct tm));

   sprintf(buf, "%02d%02d%02d", tNow.tm_hour, tNow.tm_min, tNow.tm_sec);
}//end convertTimeToString


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Convert raw time stamp to readable date string
// Design:
//-----------------------------------------------------------------------------
void LoggerCommon::convertTimeToDateString(long time, char* buf)
{
   struct tm tNow;
   struct tm *t;

   t = localtime((time_t*)&time);

   memcpy(&tNow, t, sizeof(struct tm));

   // check to make sure we're only looking at values of 0-99
   if( tNow.tm_year >= 100 )
      tNow.tm_year -= 100;

   sprintf(buf, "%02d-%02d-%02d", tNow.tm_mon+1, tNow.tm_mday, tNow.tm_year);
}//end convertTimeToDateString


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Format the logMessage into its viewable format for outputting
//              to a file or to the screen
// Design:
//-----------------------------------------------------------------------------
void LoggerCommon::formatLogMessage(LogMessage* logMessage, char* buffer)
{
   int cursor = 0;
   char* fileName = NULL;
   char time[9];

   // Convert the timestamp to a string
   LoggerCommon::convertTimeToString( logMessage->timeStamp, time );

   // Added sequence ID and timestamp string to buffer
   sprintf((buffer + cursor), "%3d %s ", logMessage->sequenceId, time);
   cursor = strlen(buffer);

   // Since the __FILE__ Macro gives the entire path to the file, go to the
   // last '\' character, then increment the pointer to get the filename
   fileName = strrchr(logMessage->sourceFilePI, '/');
   if( fileName == NULL )
   {
      // Check for both types of slashes
      fileName = strrchr( logMessage->sourceFilePI, '\\');
      if( fileName == NULL )
         fileName = logMessage->sourceFilePI;
      else
         fileName++;
   }//end if
   else
   {
      fileName++;
   }//end else

   // Check and add the log subsystem and severity
   if( (logMessage->severityLevel >= ERRORLOG) && (logMessage->severityLevel < LAST_LOG_SEVERITY) &&
       (logMessage->subsystem >= ALL_LOG_SUBSYSTEMS) && (logMessage->subsystem < MAX_LOG_SUBSYSTEM) )
      sprintf( (buffer + cursor), "%s[%s] ", logSubSystemName[logMessage->subsystem],
         severityLevelName[logMessage->severityLevel]);
   else
      sprintf( (buffer + cursor), "%s[%s] ", logSubSystemName[logMessage->subsystem], "UNKNOWN");
   cursor = strlen(buffer);

   // Output the PID of the process (and its name if its available)
   sprintf( (buffer + cursor), "(%d) ", logMessage->pid );
   cursor = strlen(buffer);

   // Add the source file name
   sprintf( (buffer + cursor), "%s ", fileName );
   cursor = strlen(buffer);

   // Add the line number of the source file
   sprintf( (buffer + cursor), "%d ", logMessage->sourceLine );
   cursor = strlen(buffer);

   // Add the log message payload. Here handle STRACELOG and TRACELOG differently
   // since we need to delete the allocated string for STRACELOG
   if (logMessage->isStringLog == true)
   {
      // Check to make sure we limit the size of the buffer.  We subtract
      // from the max length to account for the newline and NULL character that
      // need to go at the end of the message.
      if( (strlen(logMessage->logMessagePI) + cursor) < (LOG_BUFFER_SIZE - 2) )
      {
         sprintf( (buffer + cursor), logMessage->logMessagePI );
         cursor = strlen(buffer);
         sprintf( (buffer + cursor), "\n");
      }//end if
      else
      {
         // Since the string is already formatted, just do a straight byte copy into
         // the buffer for the known good size.  Then put a NULL character on the end.
         memcpy( (buffer + cursor), logMessage->logMessagePI, (LOG_BUFFER_SIZE - 2 - cursor) );
         sprintf( (buffer + LOG_BUFFER_SIZE - 2), "\n%c", '\0');
      }//end else

      // We only need to free the memory that was dynamically created. This memory gets
      // freed when the logMessage struct goes out of scope in the traceLog methods
      // So, NO need to delete it here
   }//end if
   else
   {
      sprintf( (buffer + cursor), logMessage->logMessagePI, logMessage->arg1, logMessage->arg2,
         logMessage->arg3, logMessage->arg4, logMessage->arg5, logMessage->arg6);
      cursor = strlen(buffer);
      sprintf( (buffer + cursor), "\n");
   }//end else
}//end formatLogMessage


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Provide for text wrapping functionality if necessary
// Design:
//-----------------------------------------------------------------------------
char* LoggerCommon::wrapFormattedLogText(char* buffer, char* wrapBuffer)
{
   int msgLen;
   int msgCursor = 0;
   int printCursor = 0;

   if( strlen(buffer) > 80 )
   {
      // Handle the first line separately.  Then start indenting
      sprintf( &wrapBuffer[printCursor], "%.80s", &buffer[msgCursor] );
      sprintf( &wrapBuffer[printCursor+80], "\n");

      // Make sure to account for the newline in the wrapBufferer.
      printCursor += 81;
      msgCursor += 80;

      msgLen = strlen(buffer) - 80;

      while(msgLen > 0)
      {
         sprintf( &wrapBuffer[printCursor], "     %-.75s", &buffer[msgCursor] );
         sprintf( &wrapBuffer[printCursor+80], "\n");
                                                                                                                        
         // Make sure to account for the newline in the wrapBuffer.
         printCursor += 81;
         msgCursor += 75;
                                                                                                                        
         msgLen -= 75;
      }//end while

      return wrapBuffer;
   }//end if
   // If the buffer is small enough, no need to do a copy.
   else
   {
      return buffer;
   }//end else
}//end wrapFormattedLogText


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

