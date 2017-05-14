/******************************************************************************
*
* File name:   LogProcessor.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/syslog.h>

#include <ace/Get_Opt.h>
#include <ace/Process_Semaphore.h>
#include <ace/Select_Reactor.h>
#include <ace/Sig_Adapter.h>
#include <ace/Thread_Manager.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LogProcessor.h"
#include "LoggerCommon.h"

#include "platform/common/Defines.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Log Processor"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

#define SYSLOG_FILENAME "platform_tracelogs"

// Number of seconds to wait on other processes before shutting down Log Processor
#define SHUTDOWN_DELAY 5

// Default approximate log file size (in bytes) -- currently small for debugging/development
#define DEFAULT_LOG_FILE_SIZE 200000

// Static singleton instance
LogProcessor::LogProcessor* LogProcessor::logProcessor_ = NULL;

// Static Select Reactor instance
ACE_Reactor* LogProcessor::selectReactor_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LogProcessor::LogProcessor()
             :loggerSMQueue_(LOGSM_QUEUENAME, LOGSM_QUEUEMUTEXNAME),
             outputMode_(UNKNOWN_OUTPUT_MODE),
             logFileName_(""),
             shuttingDown_(false),
             fileCheckCounter_(0),
             numberKeptLogFiles_(0),
             sizeKeptLogFiles_(DEFAULT_LOG_FILE_SIZE),
             processSemaphore_ (NULL)
{
   // Populate the static singleton instance
   logProcessor_ = this;

   // Create/map the process semaphore. Initialize it to be blocked
   processSemaphore_ = new ACE_Process_Semaphore(0, LOGQUEUE_SEMAPHORE_NAME);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LogProcessor::~LogProcessor()
{
   if (outputMode_ == SYSLOG_OUTPUT_MODE)
      closelog();
   logFileStream_.close();

   // Deallocate/remove the process semaphore
   processSemaphore_->remove();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Catch the shutdown signal and prepare to graceful stop
// Design:
//-----------------------------------------------------------------------------
void LogProcessor::catchShutdownSignal()
{
   cout << "Log Processor : Requested LogProcessor shutdown" << endl;
   LogProcessor::getInstance()->shutdown();
}//end catchShutdownSignal


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Retrieve the static singleton instance
// Design:
//-----------------------------------------------------------------------------
LogProcessor* LogProcessor::getInstance()
{
   if (logProcessor_ == NULL)
   {
      cout << "Log Processor Error : Singleton instance is NULL" << endl;
   }//end if
   return logProcessor_;
}//end getInstance


//-----------------------------------------------------------------------------
// Method Type: PRIVATE
// Description: Gracefully shutdown the Application and all of its child threads
// Design:
//-----------------------------------------------------------------------------
void LogProcessor::shutdown()
{
   // This thread should sleep for some more time to allow the log processor thread to
   // dump out any remaining log contents that were stuck in the SM log queue following
   // process shutdown
   ACE_OS::sleep(SHUTDOWN_DELAY);

   // Signal the log processor thread to exit and stop processing logs
   shuttingDown_ = true;

   // Again, pause for some time
   ACE_OS::sleep(SHUTDOWN_DELAY);

   // Flush the contents of the log queue - not sure why we have to do this, but when
   // shutdown is initiated, some logs remain in the queue--even if we pause for a bit
   // (It seems to get stuck on the mutex in 'getNextLog')
   LogMessage logMessage; 
   char* buffer = new char[LOG_BUFFER_SIZE];
   char* wrapBuffer = new char[LOG_BUFFER_SIZE + 200];
   while (loggerSMQueue_.isEmpty() == false)
   {
      // Retrieve the next log. NOTE: Here we are doing this without blocking/trying to
      // acquire the Mutex that protects the shared memory structure
      if ( loggerSMQueue_.dequeueLog(logMessage) == ERROR)
      {
         cout << "Log Processor : Error dequeuing logs for shutdown flush" << endl;
      }//end if

      // Process the contents of the log
      processLogContents(buffer, wrapBuffer, logMessage);
   }//end while
   delete [] buffer;
   delete [] wrapBuffer;

   // Clean up the log files
   if (outputMode_ == SYSLOG_OUTPUT_MODE)
   {
      closelog();
   }//end if
   else if (outputMode_ == LOGFILE_OUTPUT_MODE)
   {
      // Flush the file
      logFileStream_ << flush;
   }//end else if
   logFileStream_.close();

   cout << "Log Processor : Successfully closed the log file" << endl;

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
void LogProcessor::startReactor()
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
         cout << "Log Processor : Error getting errno string for (" << errno << ")" << endl;
      }//end if
      ostringstream ostr;
      ostr << "Reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      cout << "Log Processor : " << ostr.str().c_str() << endl;
   }//end while
}//end startReactor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize the Log Processor
// Design:
//-----------------------------------------------------------------------------
int LogProcessor::initialize(OutputModeType outputMode, char* logFile, int numberKeptLogFiles, int sizeKeptLogFiles)
{
   // Create a new ACE_Select_Reactor for the signal handling, etc to use
   selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);

   // Create an adapter to shutdown this service and all children
   signalAdapter_ = new ACE_Sig_Adapter((ACE_Sig_Handler_Ex) LogProcessor::catchShutdownSignal);

   // Specify which types of signals we want to trap
   signalSet_.sig_add (SIGINT);
   signalSet_.sig_add (SIGQUIT);
   signalSet_.sig_add (SIGTERM);

   // Register ourselves to receive signals so we can shut down gracefully.
   if (selectReactor_->register_handler (signalSet_, signalAdapter_) == ERROR)
   {
      cout << "Log Processor : Error registering for OS signals" << endl;
   }//end if

   // Startup the Reactor processing loop in a dedicated thread. NOTE: Due to compile/module
   // dependencies, this thread is not protected by the ThreadManager (so, no restarts).
   // If the reactor thread dies or becomes unstable, we 'could' manually register an AT_EXIT
   // handler inside he 'startReactor' method, but we can probably only print an error to cout
   if (ACE_Thread_Manager::instance()->spawn( (ACE_THR_FUNC) LogProcessor::startReactor, (void*) 0, THR_NEW_LWP) == ERROR)
   {
      cout << "Log Processor : Unable to spawn reactor thread" << endl;
      return ERROR;
   }//end if
   else
   {
      cout << "Log Processor : Spawning Log Processor reactor thread" << endl;
   }//end else

   // Setup shared memory
   if (loggerSMQueue_.setupQueue() == ERROR)
   {
      cout << "Log Processor: Error setting up shared memory queue" << endl;
      return ERROR;
   }//end if

   // Store the output mode information
   outputMode_ = outputMode;
   if ( (outputMode_ == LOGFILE_OUTPUT_MODE) && (logFile != NULL) )
   {
      cout << "Log Processor : Opening " << logFile << " file stream for logs" << endl;
      logFileName_ = logFile;
      numberKeptLogFiles_ = numberKeptLogFiles; 
      sizeKeptLogFiles_ = sizeKeptLogFiles;
      logFileStream_.open (logFile, ios_base::out | ios_base::app);

      if (logFileStream_.fail())
      {
         cout << "Log Processor : Failed to open file " << logFile << " for logs" << endl;
      }//end if
      // Here we could have used ACE's ofstream (and specified earlier that we should control it)
      //logFileStream_ = *ACE_Log_Msg::instance()->msg_ostream(); 
   }//end if

   // Initialize syslog if specified
   else if (outputMode_ == SYSLOG_OUTPUT_MODE)
   {
      setlogmask (LOG_UPTO (LOG_DEBUG));
      // Do a 'man syslogd'. LOG_LOCAL1 requires that we put an entry in /etc/syslog.conf:
      //    local1.*      /var/log/platform_tracelogs
      // Then re-init syslogd by: kill -SIGHUP `cat /var/run/syslogd.pid`
      openlog (SYSLOG_FILENAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
   }//end if

   // Initialize the output mechanism
   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Start the log processing loop
// Design:
//-----------------------------------------------------------------------------
void LogProcessor::processLogs()
{
   LogMessage logMessage; 
   char* buffer = new char[LOG_BUFFER_SIZE];
   char* wrapBuffer = new char[LOG_BUFFER_SIZE + 200];
   while ( (shuttingDown_ == false) || (loggerSMQueue_.isEmpty() == false) )
   {
      // Retrieve the next log. This is a blocking call while the shared memory Log queue is empty
      if (getNextLog(logMessage) == ERROR)
      {
         cout << "Log Processor: Error retrieving log from shared memory" << endl;
      }//end if

      // Process the contents of the log
      processLogContents(buffer, wrapBuffer, logMessage);
   }//end while
   delete [] buffer;
   delete [] wrapBuffer;
}//end processLogs


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve and process the contents of the next log
// Design:
//-----------------------------------------------------------------------------
void LogProcessor::processLogContents(char* buffer, char* wrapBuffer, LogMessage& logMessage)
{
   // format the log message here in this thread context
   LoggerCommon::formatLogMessage(&logMessage, buffer);

   // Don't wrap the text if we are going to pass to syslog
   char* outputBuffer = NULL;
   if (outputMode_ == SYSLOG_OUTPUT_MODE)
   {
      outputBuffer = buffer;
   }//end if
   else
   { 
      // Use a larger buffer for wrapped text (fudge factor here!)
      outputBuffer = LoggerCommon::wrapFormattedLogText(buffer, wrapBuffer);
   }//end else

   if (outputMode_ == STDOUT_OUTPUT_MODE)
   {
      // Now write the log to the selected output mechanism
      printf(outputBuffer);
      fflush(stdout);
      fflush(stderr);
   }//end if
   else if (outputMode_ == LOGFILE_OUTPUT_MODE)
   {
      logFileStream_ << outputBuffer << flush;

      // If the '-n' option was specified to enable log file rollover (so numberKeptLogFiles is not zero):
      // For every 10th log that we write to the file (in this mode), we check the size of the
      // file. If it exceeds the specified limit, then we rollover to the next file (and rename the existing ones)
      // NOTE: To tune the performance, this file size checking interval (every 10 logs) may need changing.
      if ((numberKeptLogFiles_ != 0) && (fileCheckCounter_ == 10))
      {
         off_t fileSize;
         struct stat fileStats;
         // Check the size of the file
         if (stat(logFileName_.c_str(), &fileStats))
         {
            cout << "Log Processor: Error stat'ing log file to get file size" << endl;
         }//end if
         fileSize = fileStats.st_size;

         // If we have exceeded the size, then rollover all of the files. NOTE: here, if '-z' 
         // file size option was specified, then we use that number; otherwise we use whatever
         // the default is as specified in the constructor initialization list.
         if (fileSize >= sizeKeptLogFiles_)
         {
            // Existing log file extensions get incremented, but not past the specified number 
            // of 'kept' log files (after that they get deleted)
            ifstream logFile;
            for (int i = (numberKeptLogFiles_ - 1); i > 0; i--)
            {
               // check to see if the file exists (perhaps we haven't been running long enough to 
               // accumulate all of these files yet)
               ostringstream currentFileName;
               currentFileName << logFileName_ << "." << i;
               // Test to see if the file exists
               if (!stat(currentFileName.str().c_str(), &fileStats))
               {
                  // So, the file exists. If its the last file in the 'kept' range, delete it
                  if (i == (numberKeptLogFiles_ - 1))
                  {
                     if (remove(currentFileName.str().c_str()) == ERROR)
                     {
                        cout << "Log Processor : Log file remove failed" << endl;
                     }//end if
                  }//end if
                  // Increment the log file name
                  else
                  {
                     // Incremented file name
                     ostringstream newFileName;
                     newFileName << logFileName_ << "." << (i + 1);
                     // Rename the file
                     if (rename(currentFileName.str().c_str(), newFileName.str().c_str()) == ERROR)
                     {
                        cout << "Log Processor : Log file rename failed" << endl;
                     }//end if
                  }//end else
               }//end if
            }//end for

            // If its the first/active log file, then close the active log file stream, move
            // it to a new filename (with .1 extension) and open a new file (this is the i==0 case above)
            // Flush and close
            logFileStream_ << flush;
            logFileStream_.close();
            // Build the new filename
            ostringstream newFileName;
            newFileName << logFileName_ << ".1";
            // Rename the file
            if (rename(logFileName_.c_str(), newFileName.str().c_str()) == ERROR)
            {
               cout << "Log Processor : Active log file rename failed" << endl;
            }//end if
            // Open the new/clean file
            logFileStream_.open (logFileName_.c_str(), ios_base::out | ios_base::app);
            // Check the new file stream
            if (logFileStream_.fail())
            {
               cout << "Log Processor : Failed to reopen file " << logFileName_
                    << " for logs following rollover" << endl;
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
   }//end else if
   else if (outputMode_ == SYSLOG_OUTPUT_MODE)
   {
      // Map platform severity levels to those for syslog (see /usr/include/sys/syslog.h)
      int syslogSeverity = LOG_DEBUG;
      if (logMessage.severityLevel == ERRORLOG)
         syslogSeverity = LOG_ERR;
      else if(logMessage.severityLevel == WARNINGLOG)
         syslogSeverity = LOG_WARNING;
      else if(logMessage.severityLevel == INFOLOG)
         syslogSeverity = LOG_INFO;
      else if(logMessage.severityLevel == DEBUGLOG)
         syslogSeverity = LOG_DEBUG;
      else if(logMessage.severityLevel == DEVELOPERLOG)
         syslogSeverity = LOG_DEBUG;
         
      syslog(syslogSeverity, outputBuffer);
   }//end else if

   // No need to call reset since everything gets re-assigned
   //logMessage.reset();

   memset(buffer, 0, LOG_BUFFER_SIZE);
   memset(wrapBuffer, 0, LOG_BUFFER_SIZE + 200);
}//end processLogContents


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Get the next log from the queue. This method will block while
//              the queue is empty.
// Design:
//-----------------------------------------------------------------------------
int LogProcessor::getNextLog(LogMessage& logMessage)
{
   // Block here while the queue is empty
   while (loggerSMQueue_.isEmpty() == true)
   {
      //cout << "Sleeping for 1 second while log queue is empty" << endl;
      //ACE_OS::sleep(1);

      // Instead of doing a busy wait, let's do a real wait until we are signaled
      // by an alarm/event that the queue is no longer empty. This will block as long
      // as the semaphore counter is 0; otherwise it will decrement the counter.
      // The post event will increment the counter (and thus unblock the wait)
      processSemaphore_->acquire();
   }//end while

   return loggerSMQueue_.dequeueLog(logMessage); 
}//end getNextLog


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function Type: main function for logger binary
// Description:
// Design:
//-----------------------------------------------------------------------------
int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
   char* logFile = NULL;
   int numberKeptLogFiles = 0;
   int sizeKeptLogFiles = 0;
   OutputModeType outputMode = STDOUT_OUTPUT_MODE;

   // Turn of OS limits and enable core file generation
   struct rlimit resourceLimit;
   resourceLimit.rlim_cur = RLIM_INFINITY;
   resourceLimit.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &resourceLimit);

   // Create usage string
   ostringstream usageString;
   string str4spaces = "    ";
   usageString << "Usage:\n  Logger\n" << str4spaces
               /* Process Manager/Platform specific options */
               << "-f <filename> Force output to the specified logfile\n" << str4spaces
               << "-n <number> Number of log files to preserve during rollover (when -f is specified)\n" << str4spaces
               << "-z <size> Size of each log file to allow prior to rollover (when -f is specified)\n" << str4spaces
               << "-o Force output to the OS Syslog facility\n" << str4spaces
               << "-s Force output to stderr/stdout (DEFAULT)\n" << str4spaces;
   usageString << ends;
                                                                                                                   
   // Perform our own arguments parsing -before- we pass args to the Service Configurator
   ACE_Get_Opt get_opt (argc, argv, "f:n:z:os");
   int c;
   while ((c = get_opt ()) != -1)
   {
      switch (c)
      {
         case 'o':
            // NOTE: Currently there is no way to redirect stdout/stderr to syslog, but we shouldn't
            // be using those anyway.

            cout << "Log Processor : Output selected to go to syslog facility" << endl;
            // output to Syslog facility 
            outputMode = SYSLOG_OUTPUT_MODE;
            // Tell the ACE Logger to output to stderr
            ACE_Log_Msg::instance()->open(argv[0], ACE_Log_Msg::SYSLOG, ACE_TEXT(SYSLOG_FILENAME));
            break;
         case 'f':
         {
            // Redirect stdout and stderr to the following file
            logFile = get_opt.optarg;
            cout << "Log Processor : Output selected to go to file <" << logFile << ">" << endl;
            // Test for access rights on the file
            if ((ACE_OS::access (logFile, R_OK) || ACE_OS::access (logFile, W_OK)) == ERROR)
            {
               cout << "Log Processor : Cannot read file " << logFile << endl;
               exit(1);
            }//end if

            // Use freopen to redirect; however, we could use stdio dup2 call to copy
            // ACE_STDOUT/ACE_STDERR
            // NOTE!! SHOULD this have rw permissions as well?
            if ( (freopen (logFile, "a+", stdout) == NULL) || (freopen (logFile, "a+", stderr) == NULL) )
            {
               cout << "Log Processor : Error redirecting stdout/stderr with freopen" << endl;
            }//end if

            // Redirect the ACE Logger's output to our text file. This affects ACE components that
            // use the Logger by default (such as the Service Configurator). NOTE!! If we want to
            // redirect the Platform Logger to the OS Syslog facility, then we will need to re-do
            // this configuration at that time (see ace/Log_Msg.h for flag definitions)
            //
            // NOTE: Not sure how ACE Logger handles this ofstream, don't trust it.
            // This means that we need to use a different one(handle) -or- later retrieve the
            // the handle that ACE Logger is now using.
            ostringstream aceLogFile;
            aceLogFile << logFile << ".ACE" << ends;
            ofstream ofstreamFile;
            ofstreamFile.open (aceLogFile.str().c_str(), fstream::in | fstream::out | fstream::app);
            // Let ACE take control of this ofstream -- it will be deleted/closed by ACE
            ACE_Log_Msg::instance()->msg_ostream(&ofstreamFile,1); 
            ACE_Log_Msg::instance()->set_flags(ACE_Log_Msg::OSTREAM);
            ACE_Log_Msg::instance()->clr_flags(ACE_Log_Msg::STDERR | ACE_Log_Msg::LOGGER);
            outputMode = LOGFILE_OUTPUT_MODE;
            break;
         }//end case
         case 'n':
         {
            numberKeptLogFiles = ACE_OS::atoi(get_opt.optarg);
            cout << "Log Processor : Configured to preserve " << numberKeptLogFiles << " log files for rollover" << endl;
            break;
         }//end case
         case 'z':
         {
            sizeKeptLogFiles = ACE_OS::atoi(get_opt.optarg); 
            cout << "Log Processor : Configured for log files of size " << sizeKeptLogFiles << " bytes" << endl;
            break;
         }//end case
         case 's':
            cout << "Log Processor : Output selected to go to stdout/stderr" << endl;
            // output to stdout/stderr
            outputMode = STDOUT_OUTPUT_MODE;
            // Tell the ACE Logger to output to stderr
            ACE_Log_Msg::instance()->open(argv[0]);
            break;
         case '?':
         default:
            // Save cout settings (to make the char display as ASCII)
            //ios_base::fmtflags cout_fmtflags = cout.flags();
            cout << argv[0] << " Misunderstood Option: " << c << endl;
            // Reset cout settings
            //cout.flags(cout_fmtflags);
            cout << usageString.str() << endl;
            exit(-1);
      }//end switch
   }//end while

   // Create the LogProcessor instance
   LogProcessor* logProcessor = new LogProcessor();
   if (!logProcessor)
   {
      cout << "Log Processor: Could not create Log Processor instance" << endl;
      return ERROR;
   }//end if

   // Initialize the Log Processor
   if (logProcessor->initialize(outputMode, logFile, numberKeptLogFiles, sizeKeptLogFiles) == ERROR)
   {
      cout << "Log Processor: Could not initialize the Log Processor instance" << endl;
      return ERROR;
   }//end if
   else
   {
      cout << "Logger successfully started with PID=" << ACE_OS::getpid() << endl;
   }//end else

   // Start the Log Processor's log processing loop and run forever
   logProcessor->processLogs(); 

   // Should never get here, but if so, close resources
   if (outputMode == SYSLOG_OUTPUT_MODE)
      closelog();

   cout << "Log Processor: Main Method exiting" << endl;
}//end main
