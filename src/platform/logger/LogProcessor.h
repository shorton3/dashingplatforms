/******************************************************************************
* 
* File name:   LogProcessor.h 
* Subsystem:   Platform Services 
* Description: Perform dequeuing of LogMessages from shared memory and send
*              the logs to the specified output mechanism (syslog, logfile,
*              stdout).
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_LOG_PROCESSOR_H_
#define _PLAT_LOG_PROCESSOR_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Process_Mutex.h>
#include <ace/Reactor.h>
#include <ace/Signal.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LoggerSMQueue.h"

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
 * LogProcessor performs dequeuing of LogMessages from shared memory and sends
 * the logs to the specified output mechanism (syslog, logfile, stdout). 
 * <p>
 * LogProcessor accepts the following options:
 * -f <filename> Force output to the specified logfile (provide logfile name as argument here)
 * -n <number> Number of log files to preserve during rollover (when -f is specified)
 * -z <size> Size of each log file to allow prior to rollover (when -f is specified)
 * -o Force output to the OS Syslog facility
 * -s Force output to stderr/stdout. This is the DEFAULT.
 * <p>
 * Note that when we are doing our own log file management (-f mode option specified),
 * then: a.) if '-n' option is specified, then log files will rollover where the current
 * active log file always remains <filename>, the next oldest log file will be <filename>.1,
 * and the oldest log file will be <filename>.(n - 1) -- after which, the log files will 
 * be deleted from the filesystem. Note that this style of rollover is consistent with
 * how the syslog daemon does it. b.) If '-n' is specified as well as the '-z' option,
 * then each log file will be limited to <size>. If '-n' is specified, but '-z' is
 * omitted, then a default size will be used for each log file. c.) If '-n' is omitted,
 * then log files will NOT rollover, and the specified <filename> will grow until
 * manually cleaned up (recommended for developers).
 * <p>
 * Note that when the syslog option is used, stdout/stderr do not get redirected
 * there (system limitation); however, we should not be using those anyway (use our
 * logger API instead). Also, to cause syslogd to re-initialize, perform the following:
 *    kill -SIGHUP `cat /var/run/syslogd.pid`
 * after editting /etc/syslog.conf and adding the configuration. Do a 'man syslogd'. 
 * LOG_LOCAL1 requires that we put an entry in /etc/syslog.conf:
 *    local1.*      /var/log/platform_tracelogs
 * <p>    
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Definitions of the various LogProcessor output mode types */
typedef enum
{
   UNKNOWN_OUTPUT_MODE = -1,
   LOGFILE_OUTPUT_MODE = 0,
   SYSLOG_OUTPUT_MODE = 1,
   STDOUT_OUTPUT_MODE = 2 /*Default*/
} OutputModeType;

class ACE_Process_Semaphore;

class LogProcessor
{
   public:

      /** Constructor */
      LogProcessor();

      /** Virtual Destructor */
      virtual ~LogProcessor();

      /** 
       * Initialize the Log Processor.
       * @param outputMode to file, stdout/err, or syslog
       * @param logFile the filename if -f file output mode is selected
       * @param numberKeptLogFiles number of log files to preserve during rollover (for -f file mode only)
       * @param sizeKeptLogFiles size of each log file to allow prior to rollover (for -f file mode only)
       * @returns OK on success; otherwise ERROR.
       */
      int initialize(OutputModeType outputMode, char* logFile, int numberKeptLogFiles, int sizeKeptLogFiles);

      /**
       * Start the log processing loop. This method blocks forever.
       */
      void processLogs();

      /** Catch the shutdown signal and begin the graceful shutdown */
      static void catchShutdownSignal();

   protected:

   private:

      /** Retrieve the static singleton instance of the LogProcessor */
      static LogProcessor* getInstance();

      /** Start the reactor processing thread for signal handling, etc. */
      static void startReactor();
                                                                                                                        
      /** Gracefully shutdown the LogProcessor and all of its threads */
      void shutdown();

      /**
       * Get the next log from the queue. This method will block while
       * the queue is empty.
       * @returns OK on success; otherwise ERROR
       */
      int getNextLog(LogMessage& logMessage);

      /**
       * Process the contents of each log. This is used for normal runtime
       * processing of logs as well as the log flush procedure that is performed
       * at shutdown for clearing out the queue.
       */
      void processLogContents(char* buffer, char* wrapBuffer, LogMessage& logMessage);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LogProcessor(const LogProcessor& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LogProcessor& operator= (const LogProcessor& rhs);

      /** Static singleton instance */
      static LogProcessor* logProcessor_;

      /** Shared Memory Queue for exchanging log messages between processes */
      LoggerSMQueue loggerSMQueue_;

      /** Mode for outputting log files */
      OutputModeType outputMode_;

      /** Pointer to an output file stream (if Logfile Mode is selected) */
      fstream logFileStream_;

      /** Log file name string (if Logfile Mode is selected) */
      string logFileName_;

      /** Flag for indicating when we are shutting down the process */
      bool shuttingDown_;

      /** Signal Adapter for registering signal handlers */
      ACE_Sig_Adapter* signalAdapter_;

      /** Signal Set for registering signal handlers */
      ACE_Sig_Set signalSet_;

      /** Counter for determining how many logs get processed between checking the size of the output file (-f mode) */
      int fileCheckCounter_;

      /** Number of log files to keep and maintain as part of the rollover (with -f Mode option only) */
      int numberKeptLogFiles_;

      /** Maximum size of each log file allowed before rollover */
      int sizeKeptLogFiles_;

      /** ACE Process Semaphore used to signal between enqueue/dequeue threads/processes
          when the queue is empty and non-empty */
      ACE_Process_Semaphore* processSemaphore_;

      /** ACE_Select_Reactor used for signal handling */
      static ACE_Reactor* selectReactor_;
};

#endif
