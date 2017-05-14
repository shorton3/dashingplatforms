/******************************************************************************
*
* File name:   OSResource.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/vfs.h>

#include <ace/OS.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OSResource.h"

#include "platform/logger/Logger.h"

#include "platform/datamgr/DataManager.h"
#include "platform/datamgr/DbConnectionHandle.h"
#include "platform/common/ConnectionSetNames.h"

#include "platform/common/AlarmsEvents.h"
#include "platform/faultmgr/Faults.h"

#include "platform/utilities/SystemInfo.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

#define FSTAB_LINE_LENGTH 128
#define MEMINFO_LINE_LENGTH 80

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
OSResource::OSResource()
           : previousActiveCPU_ (0),
             previousCPUIdle_ (0),
             diskHighWaterMark_ (0),
             cpuHighWaterMark_ (0),
             memoryHighWaterMark_ (0)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
OSResource::~OSResource()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize the structures for tracking resource usage
// Design:     
//-----------------------------------------------------------------------------
int OSResource::initialize()
{
   char buffer[FSTAB_LINE_LENGTH];

   // Use the contents of /etc/fstab to know which partitions needs monitoring
   FILE *fp = ACE_OS::fopen ("/etc/fstab", ACE_TEXT ("r"));
   if (fp == 0)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error opening /etc/fstab for reading",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Now populate our partition map with the valid partitions to monitor
   while (ACE_OS::fgets(buffer, sizeof buffer, fp))
   {
      char deviceName[32];
      char mountPoint[32];

      // Read out the device name and mount point name
      if (sscanf(buffer, "%s %s", deviceName, mountPoint) == 2)
      {
         // Skip over the fstab entry if it is: a cdrom, floppy, proc/shm filesystem, 
         // network filesystem, or a commented line
         if(ACE_OS::strcmp(deviceName, "none") && 
            ACE_OS::strncmp(mountPoint, "/mnt", strlen("/mnt")) &&
            !ACE_OS::strchr(deviceName, '#') &&     
            *mountPoint == '/')
         {
            // Insert partition in the map
            diskUsageMap_.insert (make_pair(mountPoint, 0));

            // Debug Log
            if (Logger::getSubsystemLogLevel(RSRCMONLOG) == DEVELOPERLOG)
            {
               ostringstream debugMsg;
               debugMsg << "Adding mountpoint (" << mountPoint << ") for monitoring" << ends;
               STRACELOG(DEBUGLOG, RSRCMONLOG, debugMsg.str().c_str()); 
            }//end if
         }//end if
      }//end if
   }//end while

   // Close the fstab file
   fclose(fp);

   // Retrieve the high water mark thresholds for alarms from the database
   DbConnectionHandle* connectionHandle = DataManager::reserveConnection(RESOURCE_MONITOR_CONNECTION);
   if (connectionHandle == NULL)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error reserving connection",0,0,0,0,0,0);
      // To allow for development, do not return
      //return ERROR;
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, RSRCMONLOG, "Successfully reserved Resource Monitor ConnectionSet Connection",0,0,0,0,0,0);
   }//end else

   // Run a query against the Alarm Inventory table
   ostringstream alarmsQuery;
   alarmsQuery << "Select AlarmCode, HighWaterMark from platform.AlarmsInventory where AlarmCode in ("
               << ALARM_OS_RESOURCE_DISKUSAGE << "," << ALARM_OS_RESOURCE_CPUUSAGE << "," 
               << ALARM_OS_RESOURCE_MEMORYUSAGE << ");" << ends;

   // Execute the query against the database
   if (connectionHandle->executeCommand(alarmsQuery.str().c_str()) == ERROR)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Alarm threshold query execution failed",0,0,0,0,0,0);
      // To allow for development, do not return
      //connectionHandle->closeCommandResult();
      //DataManager::releaseConnection(connectionHandle);
      //return ERROR;
   }//end if

   TRACELOG(DEBUGLOG, RSRCMONLOG, "Preparing to update alarm thresholds with database values",0,0,0,0,0,0);

   int columnCount = connectionHandle->getColumnCount();
   int rowCount = connectionHandle->getRowCount();
   // Do some checks to see if the number of columns returned is zero! This indicates a schema error!
   if (columnCount == 0)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "No columns returned in result set, possible schema error",0,0,0,0,0,0);
      // To allow for development, do not return
      //connectionHandle->closeCommandResult();
      //DataManager::releaseConnection(connectionHandle);
      //return ERROR;
   }//end if
   else if (rowCount == 0)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "No rows returned in result set, default datafill missing",0,0,0,0,0,0);
      // To allow for development, do not return
      // To allow for development, do not return
      //connectionHandle->closeCommandResult();
      //DataManager::releaseConnection(connectionHandle);
      //return ERROR;
   }//end if
   else
   {
      // Get the column index for highwatermark
      int highWaterColumnIndex = connectionHandle->getColumnIndex("HighWaterMark");
      int alarmCodeIndex = connectionHandle->getColumnIndex("AlarmCode");

      // Set the Alarm threshold values
      for (int row = 0; row < rowCount; row++)
      {
         int highWaterMark = connectionHandle->getIntValueAt(row, highWaterColumnIndex);
         int alarmCode = connectionHandle->getShortValueAt(row, alarmCodeIndex);

         switch (alarmCode)
         {
            case ALARM_OS_RESOURCE_DISKUSAGE:
            {
               diskHighWaterMark_ = highWaterMark;
               break;
            }//end case
            case ALARM_OS_RESOURCE_CPUUSAGE:
            {
               cpuHighWaterMark_ = highWaterMark;
               break;
            }//end case
            case ALARM_OS_RESOURCE_MEMORYUSAGE:
            {
               memoryHighWaterMark_ = highWaterMark;
               break;
            }//end case 
            default:
            {
               TRACELOG(ERRORLOG, RSRCMONLOG, "Unknown alarmcode received for high water mark configuration: %d",alarmCode,0,0,0,0,0);
               break;
            }//end default
         }//end switch 
      }//end for
   }//end else

   // Close the results, release the connection
   connectionHandle->closeCommandResult();
   DataManager::releaseConnection(connectionHandle);

   TRACELOG(DEBUGLOG, RSRCMONLOG, "Resource Monitor alarm thresholds set: Disk(%d%), CPU(%d%), Memory(%d%)",
      diskHighWaterMark_, cpuHighWaterMark_, memoryHighWaterMark_, 0, 0, 0);

   return OK;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Check disk usage stats
// Design:
//-----------------------------------------------------------------------------
int OSResource::checkDiskUsage()
{
   // For each mount point in the diskUsageMap_, gather usage statistics
   DiskUsageMapType::iterator iterator;

   // Generate an event report (Probably want to remove this later as this is NOT very informational)
   REPORTEVENT(EVENT_REPORT_OS_RESOURCE_DISK_CHECK, OS_RESOURCE, 1); // Could use NEID as the managed object instance??

   for (iterator = diskUsageMap_.begin(); iterator != diskUsageMap_.end(); iterator++)
   {
      struct statfs fsInfo;
      const char* mountPoint = (iterator->first).c_str();
      // Retrieve the runtime filesystem statistics
      if (statfs(mountPoint, &fsInfo) == ERROR)
      {
         ostringstream ostr;
         ostr << "Error getting filesystem info for " << mountPoint << ends;
         STRACELOG(ERRORLOG, RSRCMONLOG, ostr.str().c_str());
         continue;
      }//end if

      // Get a raw ratio value
      double usageRatio = 100.0 * (double)(fsInfo.f_blocks - fsInfo.f_bfree) / 
                          (double)(fsInfo.f_blocks - fsInfo.f_bfree + fsInfo.f_bavail);
      // Convert the usage ratio to an integer
      unsigned int usageValue = (unsigned int)usageRatio + 1;

      // Check the Inode usage as well
      double inodesUsed = (fsInfo.f_files - fsInfo.f_ffree);
      double inodesFree = fsInfo.f_ffree;
      double totalInodes = inodesUsed + inodesFree;
      double inodeUsageRatio = 100 * (inodesUsed / totalInodes);
      unsigned int inodeUsageValue = (unsigned int)inodeUsageRatio + 1;

      ostringstream ostr;
      ostr << "Found mountPoint " << mountPoint << " to have usage " << usageValue << " percent and inode usage "
           << inodeUsageValue << " percent" << ends;
      STRACELOG(DEBUGLOG, RSRCMONLOG, ostr.str().c_str()); 

      // Here we need to store the disk and inode usage values for historical
      // archives (for offline performance analysis)



// DEBUG -----------------------------
      // Provide a hack that will generate an alarm and then clear it the next few times around
      static int debugCounter = 0;
      if (debugCounter == 0)
      {
         debugCounter++;
         usageValue = 99;
      }//end if
      else
      {
         if (debugCounter >= 5)
         {
            debugCounter = 0;
         }//end if
         debugCounter++;
      }//end else
// DEBUG END -------------------------

      // Here is where we raise an alarm if the usage value is high
      // NEED TO RETRIEVE THIS FROM DATABASE
      if (usageValue >= diskHighWaterMark_)
      {
         // Raise an Alarm
//         RAISEALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_DISKUSAGE, OS_RESOURCE, 1);

// REMOVE THIS LATER -- TEST for alarm storms (and toggling)
for (int i = 0; i < 10; i++)
{
   RAISEALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_DISKUSAGE, OS_RESOURCE, 1);
   sleep(1);
   CLEARALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_DISKUSAGE, OS_RESOURCE, 1);
   sleep(1);
}//end for


      }//end if
      else if (inodeUsageValue >= diskHighWaterMark_)
      {
         // Raise an Alarm
         RAISEALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_DISKUSAGE, OS_RESOURCE, 1);
      }//end else if
      else
      {
         // Possibly clear an outstanding Alarm
         CLEARALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_DISKUSAGE, OS_RESOURCE, 1);
      }//end if
   }//end for
   return OK;
}//end checkDiskUsage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Check CPU usage based on the stats in the Linux
//              /proc filesystem
// Design:
//-----------------------------------------------------------------------------
int OSResource::checkCPUUsage()
{
   FILE *fp;

   // Retrieve the Cpu Usage Details
   fp = ACE_OS::fopen("/proc/stat","r");
   if (fp == NULL)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error opening /proc/stat file for statistics gathering",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Linux provides cumulative counts for the number of jiffies spent in user,
   // nice, system, and idle CPU mode since the system was booted, or since the
   // jiffy count last wrapped round. A jiffy is a proverbial short amount of
   // time, which is 1/100 second on most CPUs (1/1024 on Alphas).
   //
   // The four jiffy counts are given in the /proc/stat file, on a line 
   // beginning with the word cpu. On multiprocessor systems, the counts are given
   // for each cpu, each on a line beginning with the word cpun, where n is the
   // zero-based CPU number, and the line starting with cpu contains the sums 
   // for all processors.
   //
   // The duration of a jiffy, and the cumulative number of jiffies, are not much
   // use to us; what is important is the number of jiffies spent in each mode
   // since the last time we had a look. We calculate the differences for each
   // of the modes, and express each difference as a percentage of the total difference.
   unsigned long cpuUser = 0;
   unsigned long cpuNice = 0;
   unsigned long cpuSystem = 0;
   unsigned long cpuIdle = 0;

   // Parse the CPU usage line
   if (fscanf(fp, "cpu %lu %lu %lu %lu", &cpuUser, &cpuNice, &cpuSystem, &cpuIdle) != 4)
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error reading cpu usage stats",0,0,0,0,0,0);
      fclose(fp);
      return ERROR;
   }//end if

   // Close the file
   fclose(fp);

   // Calculate the total cpu usage
   unsigned long activeCPU = cpuUser + cpuNice + cpuSystem;
   unsigned long totalCPU = activeCPU + cpuIdle - previousActiveCPU_ - previousCPUIdle_;

   // Calculate the Ratio
   unsigned short cpuUsage = 0;
   // Make sure that this is NOT the first time through, if so, set to zero
   if (totalCPU)
   {
      cpuUsage = (unsigned long)(100.0 * (double)(activeCPU - previousActiveCPU_ + 0.5) / (double) totalCPU);
      if (cpuUsage > 100)
         cpuUsage = 100;
   }//end if
   else
   {
      cpuUsage = 0;
   }//end else

   TRACELOG(DEBUGLOG, RSRCMONLOG, "CPU usage is measured to be %d percent", cpuUsage,0,0,0,0,0);

   // Generate an event report (Probably want to remove this later as this is NOT very informational)
   REPORTEVENT(EVENT_REPORT_OS_RESOURCE_CPU_CHECK, OS_RESOURCE, 1); /** Could use NEID as the managed object instance?? */

   // Store the current CPU measurement to be used in the next iteration
   previousActiveCPU_ = activeCPU;
   previousCPUIdle_ = cpuIdle;

   // Here we need to store the CPU usage values for historical
   // archives (for offline performance analysis)



   // Here is where we raise an alarm if the usage value is high
   if (cpuUsage >= cpuHighWaterMark_)
   {
      // Raise an Alarm
      RAISEALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_CPUUSAGE, OS_RESOURCE, 1);
   }//end if
   else
   {
      // Possibly clear an outstanding Alarm
      CLEARALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_CPUUSAGE, OS_RESOURCE, 1);
   }//end else
   return OK;
}//end checkCPUUsage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Check Memory usage based on the stats in the Linux
//              /proc filesystem
// Design:
//-----------------------------------------------------------------------------
int OSResource::checkMemoryUsage()
{
   char fileBuffer[MEMINFO_LINE_LENGTH];
   // Open and test the file
   ifstream memInfoFile ("/proc/meminfo");
   if (!memInfoFile.is_open())
   {
      TRACELOG(ERRORLOG, RSRCMONLOG, "Error opening /proc/meminfo file for statistics gathering",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Memory statistics variables from /proc/meminfo
   unsigned long memTotal = 0;
   unsigned long memFree = 0;
   unsigned long memBuffers = 0;
   unsigned long memCached = 0;
   unsigned long swapTotal = 0;
   unsigned long swapFree = 0;
   unsigned long swapCached = 0;

   // Read the entire file contents and parse it
   while (!memInfoFile.eof())
   {
      memInfoFile.getline(fileBuffer, MEMINFO_LINE_LENGTH);

      // If the fileBuffer contains a MemTotal
      if (strncmp(fileBuffer, "MemTotal:", strlen("MemTotal:"))==0)
      {
         sscanf(fileBuffer, "MemTotal: %lu", &memTotal);
      }//end if
      else if (strncmp(fileBuffer, "MemFree:", strlen("MemFree:")) == 0)
      {
         sscanf(fileBuffer, "MemFree: %lu", &memFree);
      }//end else if
      else if (strncmp(fileBuffer, "Buffers:", strlen("Buffers:")) == 0)
      {
         sscanf(fileBuffer, "Buffers: %lu", &memBuffers);
      }//end else if
      else if (strncmp(fileBuffer, "Cached:", strlen("Cached:")) == 0)
      {
         sscanf(fileBuffer, "Cached: %lu", &memCached);
      }//end else if
      else if (strncmp(fileBuffer, "SwapTotal:", strlen("SwapTotal:")) == 0)
      {
         sscanf(fileBuffer, "SwapTotal: %lu", &swapTotal);
      }//end else if
      else if (strncmp(fileBuffer, "SwapFree:", strlen("SwapFree:")) == 0)
      {
         sscanf(fileBuffer, "SwapFree: %lu", &swapFree);
      }//end else if
      else if (strncmp(fileBuffer, "SwapCached:", strlen("SwapCached:")) == 0)
      {
         sscanf(fileBuffer, "SwapCached: %lu", &swapCached);
      }//end else if
   }//end while

   // Close the file
   memInfoFile.close();

   // Calculate the memory usage. Here we perform the memory usage calculations as follows:
   //
   //    From the fields taken from /proc/meminfo
   //    NOTE: This calculation should agree with the output of the 'free' utility (2nd output line)
   //    Memory Used = MemTotal - MemFree - Buffers - Cached
   //
   //    Also perform Swap calculation:
   //    Swap Used = SwapTotal - SwapFree - SwapCached
   //
   unsigned long physicalMemoryUsed = memTotal - memFree - memBuffers - memCached;
   unsigned int physicalMemoryRatio = (physicalMemoryUsed / memTotal) * 100;
   // Normalize the ratio value
   if (physicalMemoryRatio > 100)
   {
      physicalMemoryRatio = 100;
   }//end if

   TRACELOG(DEBUGLOG, RSRCMONLOG, "Physical Memory usage is measured to be %d percent", physicalMemoryRatio, 0,0,0,0,0);

   unsigned long swapMemoryUsed = swapTotal - swapFree - swapCached;
   unsigned int swapMemoryRatio = (swapMemoryUsed / swapTotal) * 100;
   // Normalize the ratio value
   if (swapMemoryRatio > 100)
   {
      swapMemoryRatio = 100;
   }//end if

   TRACELOG(DEBUGLOG, RSRCMONLOG, "Swap Memory usage is measured to be %d percent", swapMemoryRatio, 0,0,0,0,0);

   // Generate an event report (Probably want to remove this later as this is NOT very informational)
   REPORTEVENT(EVENT_REPORT_OS_RESOURCE_MEMORY_CHECK, OS_RESOURCE, 1); /** Could use NEID as the managed object instance?? */

   // Here we need to store both the Physical and Swap Memory usage values for historical
   // archives (for offline performance analysis)



   // Here is where we raise an alarm if the usage value is high. We ONLY WANT to consider the physical
   // memory value when raising an alarm--this means that we want to raise an alarm BEFORE SWAP memory
   // starts to get used (so don't factor in the amount of free swap memory into the physical memory value).
   if (physicalMemoryRatio >= memoryHighWaterMark_)
   {
      // Raise an Alarm
      RAISEALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_MEMORYUSAGE, OS_RESOURCE, 1);
   }//end if
   else
   {
      // Possibly clear an outstanding Alarm
      CLEARALARM(SystemInfo::getLocalNEID().c_str(), ALARM_OS_RESOURCE_MEMORYUSAGE, OS_RESOURCE, 1);
   }//end else
   return OK;
}//end checkMemoryUsage


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

