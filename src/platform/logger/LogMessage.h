/******************************************************************************
* 
* File name:   LogMessage.h 
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

#ifndef _PLAT_LOG_MESSAGE_H_
#define _PLAT_LOG_MESSAGE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Based_Pointer_T.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

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
 * Log Message structure encapsulates the information for a single
 * log entry so that it can be enqueued by the originating application
 * and then dequeued by the Log Processor process for transfer to the
 * selected output media (logfile, syslog, etc.)
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Shared Memory Initialization parameters */
#define LOGSM_QUEUENAME          "LogSMQueue"
#define LOGSM_QUEUEMUTEXNAME     "LogSMQueueMutex"
#define LOGQUEUE_SEMAPHORE_NAME  "LogSemaphore"

#define LOG_BUFFER_SIZE 1024
/** From stdio.h and bits/stdio_lim.h, FILENAME_MAX is set to 4096 chars, but for
    practicality, we will truncate source filenames greater than 100 chars
    (remember that the __FILE__ macro gives the absolute path which has to be stripped */
#define LOG_SOURCE_FILE_SIZE 100 


struct LogMessage
{
   /** 
    * Indicates this is a String Log (stracelog) message. 
    *
    * IMPORTANT!!!! Whenever creating a LogMessage structure anywhere (stack or heap),
    * this boolean must be set. It cannot be passed in the constructor because a default
    * constructor is needed for template instantiation
    */
   bool isStringLog;
   /** Sequence in which logs are enqueued for processing */
   unsigned char sequenceId;
   /** Log subsystem type (MSGMGR, OPM...) */
   int subsystem;
   /** Log severity level (DEBUG, ERROR...) */
   int severityLevel;
   /** OS Process ID originating the log message */
   int pid;
   /** Source file line where the log originated */
   int sourceLine;
   /** Time stamp for log origination */
   long timeStamp;
   /** Source file name where the log originated */
   char sourceFile[LOG_SOURCE_FILE_SIZE];
   /** Position Independent ACE Pointer to the source file name. NOTE: there is no allocated memory here */
   ACE_Based_Pointer_Basic<char> sourceFilePI;
   /** Log Message payload string */
   char logMessage[LOG_BUFFER_SIZE];
   /** Position Independent ACE Pointer to the log Message payload string. NOTE: there is no allocated memory here */
   ACE_Based_Pointer_Basic<char> logMessagePI;
   /** Arguments passed in for automatic formatting into the log */
   #if __WORDSIZE == 64
   long arg1;
   long arg2;
   long arg3;
   long arg4;
   long arg5;
   long arg6;
   #else // WORDSIZE is 32bits
   int arg1;
   int arg2;
   int arg3;
   int arg4;
   int arg5;
   int arg6;
   #endif   

   /** Default Constructor */
   LogMessage();

   /** Copy Constructor */
   LogMessage( const LogMessage& rhs );

   /** Destructor */
   ~LogMessage(void);

   /** Reset data members */
   void reset(void);

   /** Overloaded assignment operator */
   LogMessage& operator= ( const LogMessage& rhs );

   /** Overloaded equality operator */
   bool operator== (const LogMessage& rhs);

};//end LogMessage structure

#endif
