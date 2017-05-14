/******************************************************************************
* 
* File name:   OSResource.h 
* Subsystem:   Platform Services 
* Description: This class provides the mechanisms for monitoring OS-controlled
*              resources including Memory, CPU, and Disk usages.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_OS_RESOURCE_H_
#define _PLAT_OS_RESOURCE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>
#include <map>

using namespace std;

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
 * OSResource provides the mechanisms for monitoring OS-controlled
 * resources including Memory, CPU, and Disk usages.
 * <p>
 * With regard to measuring CPU usage on Linux:
 * Linux provides cumulative counts for the number of jiffies spent in user,
 * nice, system, and idle CPU mode since the system was booted, or since the
 * jiffy count last wrapped round. A jiffy is a proverbial short amount of
 * time, which is 1/100 second on most CPUs (1/1024 on Alphas).
 * <p> 
 * The four jiffy counts are given in the /proc/stat file, on a line
 * beginning with the word cpu. On multiprocessor systems, the counts are
 * given for each cpu, each on a line beginning with the word cpun, where 
 * n is the zero-based CPU number, and the line starting with cpu contains
 * the sums for all processors.
 * <p> 
 * The duration of a jiffy, and the cumulative number of jiffies, are not much
 * use to us; what is important is the number of jiffies spent in each mode
 * since the last time we had a look. We calculate the differences for each
 * of the modes, and express each difference as a percentage of the total
 * difference.
 * <p>
 * For measuring memory usage, we inspect the fields taken from /proc/meminfo.
 * Note that this calculation should agree with the output of the 'free' 
 * utility (2nd output line). The calculations are as follows:
 *   Memory Used = MemTotal - MemFree - Buffers - Cached
 * Also perform Swap calculation:
 *   Swap Used = SwapTotal - SwapFree - SwapCached
 * <p>
 * Disk usage values are gathered by comparing /etc/fstab with the output
 * from the statfs library call.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class OSResource
{
   /** Map used for storing disk usage statistics */
   typedef map< string, unsigned short, less<string> > DiskUsageMapType;

   public:

      /** Constructor */
      OSResource();

      /** Virtual Destructor */
      virtual ~OSResource();

      /** 
       * Initialize the structures for tracking resource usage
       * @returns ERROR upon failure; otherwise OK
       */
      int initialize();

      /** 
       * Check disk usage for all detected filesystem partitions
       * @returns ERROR upon failure; otherwise OK
       */
      int checkDiskUsage();

      /**
       * Check CPU usage based on statistics in the Linux proc filesystem
       * @returns ERROR upon failure; otherwise OK
       */
      int checkCPUUsage();

      /**
       * Check memory usage based on statistics in the Linux proc filesystem
       * @returns ERROR upon failure; otherwise OK
       */
      int checkMemoryUsage();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      OSResource(const OSResource& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      OSResource& operator= (const OSResource& rhs);

      /** Map for storing disk usage statistics */
      DiskUsageMapType diskUsageMap_;

      /** Previous CPU usage stats for calculating the delta */
      unsigned long previousActiveCPU_;

      /** Previous CPU usage stats for calculating the delta in the Idle time */
      unsigned long previousCPUIdle_;

      /** Threshold value for raising an alarm */
      unsigned int diskHighWaterMark_;

      /** Threshold value for raising an alarm */
      unsigned int cpuHighWaterMark_;

      /** Threshold value for raising an alarm */
      unsigned int memoryHighWaterMark_;

};

#endif
