/******************************************************************************
*
* File name:   Alarms.h
* Subsystem:   Platform Services
* Description: Contains definitions for alarms and alarm related parameters.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_ALARMS_H_
#define _PLAT_ALARMS_H_


/** Alarm Severity -- includes a Clear Alarm and the Informational Event Report */
typedef enum 
{ 
   ALARM_INDETERMINATE = 0,
   ALARM_CRITICAL,
   ALARM_MAJOR,
   ALARM_MINOR,
   ALARM_WARNING,
   ALARM_CLEAR,
   INFORMATIONAL_EVENT,  /** This type is synonymous with an event report type */
   ALARM_LAST_SEVERITY
} AlarmSeverityType;


/** Alarm Category Types as specified by ITU-T X.733 */
typedef enum
{
   COMMUNICATIONS_ALARM = 0,
   QUALITY_OF_SERVICE_ALARM,
   PROCESSING_ERROR_ALARM,
   EQUIPMENT_ALARM,
   ENVIRONMENTAL_ALARM,
   ALARM_LAST_CATEGORY
} AlarmCategoryType;


/**
 * First six bits are to identify the subsystem, and the last 10 bits are to
 * identify the alarm. Each alarm is unique across the system. To add a new
 * alarm within a subsystem declare it before the _LAST entry within that
 * subsystem. To add a new subsystem create a block with a _BASE and _LAST
 * entry and add alarms within that block. Use the first available one.
 * Increment the value by the appropriate index from the subsystem base.
 */


/** 
 * Enumerated list of Alarm Codes.
 * This list MUST MATCH the default datafill of the database since we assign the
 * alarm severity, alarm description, and resolution fields from the default schema
 * (so that its not hardcoded or controlled by the applications).
 */
typedef enum
{
   /** Resource Monitor -- OS related alarms */
   ALARM_OS_RESOURCE_BASE =               0x0000,

   ALARM_OS_RESOURCE_DISKUSAGE =          ALARM_OS_RESOURCE_BASE + 1,
   ALARM_OS_RESOURCE_CPUUSAGE =           ALARM_OS_RESOURCE_BASE + 2,
   ALARM_OS_RESOURCE_MEMORYUSAGE =        ALARM_OS_RESOURCE_BASE + 3,

   ALARM_OS_RESOURCE_LAST =               ALARM_OS_RESOURCE_BASE + 1023, // 0x03FF

   /** Data Manager -- Database related alarms */
   ALARM_DATA_MANAGER_BASE =              0x0400,
    
   ALARM_DATA_MANAGER_CONNECTFAILED =     ALARM_DATA_MANAGER_BASE + 1,
    
   ALARM_DATA_MANAGER_LAST =              ALARM_DATA_MANAGER_BASE + 1023, // 0x07FF

   ALARM_CODE_LAST
} AlarmCodeType;


/** 
 * Enumerated list of Event Reports.
 * This list MUST MATCH the default datafill of the database.
 */
typedef enum
{
   /** Resource Monitor Event Reports */
   EVENT_REPORT_OS_RESOURCE_BASE =                    0x0000,

   EVENT_REPORT_OS_RESOURCE_DISK_CHECK =              EVENT_REPORT_OS_RESOURCE_BASE + 1,
   EVENT_REPORT_OS_RESOURCE_MEMORY_CHECK =            EVENT_REPORT_OS_RESOURCE_BASE + 2,
   EVENT_REPORT_OS_RESOURCE_CPU_CHECK =               EVENT_REPORT_OS_RESOURCE_BASE + 3,

   EVENT_REPORT_OS_RESOURCE_LAST =                    EVENT_REPORT_OS_RESOURCE_BASE + 1023, //0x03FF

   /** Data Manager Event Reports */
   EVENT_REPORT_DATAMANAGER_BASE =                    0x0400,

   EVENT_REPORT_DATAMANAGER_CONNECTION_CHECK =        EVENT_REPORT_DATAMANAGER_BASE + 1,

   EVENT_REPORT_DATAMANAGER_LAST =                    EVENT_REPORT_DATAMANAGER_BASE + 1023, // 0x07FF


   EVENT_REPORT_LAST
} EventCodeType;

/** Enumerated list of the managed object types that can raise alarms */
typedef enum
{
   OS_RESOURCE,
   DATABASE,


   LAST_MANAGED_OBJECT   
} ManagedObjectType;


#endif
