/******************************************************************************
*
* File name:   Logger.h
* Subsystem:   Platform Services
* Description: Implements the main API interface for logging trace logs.
*              This interface implements a high performance queuing mechanism
*              for handling alarms.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/
                                                                                                                
#ifndef _PLAT_LOGGER_H_
#define _PLAT_LOGGER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

/** Needed for Log Severity and Subsystem Declarations */
#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Use C Linkage to prevent problems with the MACROS being name mangled
//#ifdef __cplusplus
//extern "C" {
//#endif

// Need to test if we are 64bit or 32bit, as we will need to pass in pointer arguments
// to our logs. Using preprocessor macros here is ugly, but on some platforms, they will
// frown on passing long types around everywhere, and we need to support long==pointer
// the 64bit platforms and int==pointer on the 32bit platforms
#if __WORDSIZE == 64

/** This is the programmatical interface to the logger; this is for static strings */
#define TRACELOG(level, subSystem, msg, arg1, arg2, arg3, arg4, arg5, arg6)               \
   if( level <= Logger::getSubsystemLogLevel( (subSystem) ) )                             \
      Logger::traceLog( (subSystem), (level), (int)(getpid()), __FILE__, __LINE__, (msg), \
         (long)(arg1), (long)(arg2), (long)(arg3), (long)(arg4), (long)(arg5), (long)(arg6) ); 

/** Since the getpid function call is not good to call in high performance code, just pass 0 */
#define TRACELOGLITE(level, subSystem, msg, arg1, arg2, arg3, arg4, arg5, arg6)           \
   if( level <= Logger::getSubsystemLogLevel( (subSystem) ) )                             \
      Logger::traceLog( (subSystem), (level), 0, __FILE__, __LINE__, (msg),               \
         (long)(arg1), (long)(arg2), (long)(arg3), (long)(arg4), (long)(arg5), (long)(arg6) );

#else // We are 32bit, so pointer and int are same size

/** This is the programmatical interface to the logger; this is for static strings */
#define TRACELOG(level, subSystem, msg, arg1, arg2, arg3, arg4, arg5, arg6)               \
   if( level <= Logger::getSubsystemLogLevel( (subSystem) ) )                             \
      Logger::traceLog( (subSystem), (level), (int)(getpid()), __FILE__, __LINE__, (msg), \
         (int)(arg1), (int)(arg2), (int)(arg3), (int)(arg4), (int)(arg5), (int)(arg6) ); 

/** Since the getpid function call is not good to call in high performance code, just pass 0 */
#define TRACELOGLITE(level, subSystem, msg, arg1, arg2, arg3, arg4, arg5, arg6)           \
   if( level <= Logger::getSubsystemLogLevel( (subSystem) ) )                             \
      Logger::traceLog( (subSystem), (level), 0, __FILE__, __LINE__, (msg),               \
         (int)(arg1), (int)(arg2), (int)(arg3), (int)(arg4), (int)(arg5), (int)(arg6) );

#endif

/** For non-static strings that must be copied before they fall off of the stack and get deleted. */
#define STRACELOG(level, subSystem, msg)                                                  \
   if( level <= Logger::getSubsystemLogLevel( (subSystem) ) )                             \
      Logger::straceLog( (subSystem), (level), (int)(getpid()), __FILE__, __LINE__, (msg) );

//#ifdef __cplusplus
//}
//#endif

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

class ACE_Process_Semaphore;
class LogMessage;
class LoggerSMConfig;
class LoggerSMQueue;
                                                                                                                        
// For C++ class declarations, we have one (and only one) of these access
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors/destructors first)
// 5) instance data

/**
 * Logger provides the main logging mechanism and API to the developers.
 * <p>
 * Developers use the Log MACROS to issue logs. These macros hide the API
 * mechanism for enqueuing the logs for output. In this way, the logger
 * subsystem can be transparently modified to send output to a.) stdout,
 * b.) the Linux syslog facility, or even c.) the ACE logger framework
 * without impact to the application developers.
 * <p>
 * The key goal for implementing and maintaining this logger is to allow
 * applications to issue trace logs with the MINIMAL amount of buffer
 * copying possible. This is the key motivator behind supporting a 
 * custom implementation.
 *
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class Logger 
{
   public:
      
      /**
       * Destructor
       */
      ~Logger();

      /**
       * Method to handle enqueuing of the log messages.
       * <p>
       * Enqueue a log message for output
       * Memory Management here: p_logMessage is assumed to be statically
       * allocated memory when this method is used (printf style formatting
       * arguments are passed in as well). For this reason, we only need
       * to make a copy of the buffer for shared memory allocation.
       * The 'logMessage' struct that we create on the first line
       * here will be deleted (including its logMessage buffer--deleted
       * by its destructor) when this method goes out of scope.
       * 
       * A performance optimization for this logger should be to create pool of
       * logMessage structs on the heap and reuse them:  reserve one here and
       * fill its contents, then pass it into the log queue where it
       * gets deleted on the other side once it is output to where-ever.
       * NOTE: this pool should not use OPM (circular dependency).
       */
#if __WORDSIZE == 64
      static void traceLog(LogEntrySubSystemType subsystem,
         LogEntrySeverityType severity, int pid, const char* sourceFile,
         int sourceLine, const char* p_logMessage, long arg1, long arg2, long arg3,
         long arg4, long arg5, long arg6);
#else // WORDSIZE is 32bits
      static void traceLog(LogEntrySubSystemType subsystem,
         LogEntrySeverityType severity, int pid, const char* sourceFile,
         int sourceLine, const char* p_logMessage, long arg1, long arg2, long arg3,
         long arg4, long arg5, long arg6);
#endif

      /**
       * Method to handle enqueuing of the log messages
       * <p>
       * Enqueue a string log message for output (make a copy before
       * inserting in the queue.
       * Memory Management here: p_logMessage gets deleted when the calling
       * method goes out of scope since it was probably a stack variable
       * there. This is why we make a copy of p_logMessage before enqueuing
       * it to our Log Server. The 'logMessage' struct that we create on
       * the first line here will be deleted (including its logMessage
       * buffer--deleted by its destructor) when this method goes out of scope.
       * 
       * A performance optimization for this logger should be to create pool of
       * logMessage structs on the heap and reuse them:  reserve one here and
       * fill its contents, then pass it into the log queue where it
       * gets deleted on the other side once it is output to where-ever.
       * NOTE: this pool should not use OPM (circular dependency).
       */
      static void straceLog(LogEntrySubSystemType subsystem, 
         LogEntrySeverityType severity, int pid, const char* sourceFile,
         int sourceLine, const char* p_logMessage);

      /**
       * Method to retrieve the current logging severity level for a particular subsystem
       * @returns int severityLevel
       */
      static LogEntrySeverityType getSubsystemLogLevel(LogEntrySubSystemType subsystem); 

      /**
       * Method to set subsystem log severity levels. This method controls the allowable logs output.
       */
      static void setSubsystemLogLevel(LogEntrySubSystemType subsystem, LogEntrySeverityType severityLevel);

      /**
       * Return a singleton instance
       */
      static Logger* getInstance();

      /**
       * Perform initialization of shared memory queue, etc.
       * @param sendOutputToLocal If true, output is redirected to local stdout/stderr
       *    rather than enqueued for the LogProcessor process. This is for developer
       *    debugging.
       * @returns OK on success; otherwise ERROR
       */
      int initialize(bool sendOutputToLocal);

   protected:

   private:

      /**
       * Constructor
       */
      Logger();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      Logger(const Logger& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      Logger& operator= (const Logger& rhs);

      /**
       * Return a pointer to the Shared Memory queue
       */
      LoggerSMQueue* getQueue();

      /** Counter to track log sequence number */
      static unsigned short logSequenceId_;

      /** Array of currently set subsystem log severity levels */
      static unsigned char* logSubSystemLevel_;

      /** Singleton instance */
      static Logger* loggerInstance_;

      /** Shared Memory Queue for exchanging log messages between processes */
      LoggerSMQueue* loggerSMQueue_;

      /** Shared Memory structure for sharing Logger configuration data (eg. log levels */
      LoggerSMConfig* loggerSMConfig_;

      /** ACE Process Semaphore used to signal between enqueue/dequeue threads/processes
          when the queue is empty and non-empty */
      static ACE_Process_Semaphore* processSemaphore_;

      /** Flag for determining if enqueue to Log Processor is bypassed and
          output is sent to stdout/stderr for Developer debugging */
      static bool sendOutputToLocal_;

};

#endif

