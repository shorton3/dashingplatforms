/******************************************************************************
*
* File name:   Defines.h
* Subsystem:   Platform Services
* Description: Contains common defines to be used by all components of the 
*              system.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/
                                                                                                                          
#ifndef _PLAT_COMMON_DEFINES_H_
#define _PLAT_COMMON_DEFINES_H_


#if !defined(OK) || (OK!=0)
#define OK 0
#endif

#if !defined(ERROR) || (ERROR!=-1)
#define ERROR -1
#endif

#if !defined(FALSE) || (FALSE!=0)
#define FALSE 0
#endif

#if !defined(TRUE) || (TRUE!=1)
#define TRUE 1
#endif

/** Default Uninitialized Values */
#define UNKNOWN_SHELF_NUMBER -1
#define UNKNOWN_SLOT_NUMBER -1
#define UNKNOWN_NEID "000000000"
#define UNKNOWN_IP_ADDRESS "0.0.0.0"
#define UNKNOWN_PORT_NUMBER 0

// ntohll: converts a 64-bit integer to host byte order.
#define ntohll(x) (((long long)(ntohl((int)((x << 32) >> 32))) << 32) | (unsigned int)ntohl(((int)(x >> 32)))) 
// ntonll: converts a 64-bit integer to network byte order. 
#define htonll(x) ntohll(x)

// Create our own is_multicast test since the one in ACE (INET_Addr) is broken
#define IN_CLASSD(a) ((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a) IN_CLASSD(a)

/** Max supported message length for MsgMgr framework */
#define MAX_MESSAGE_LENGTH 1024

/** Definitions for Log Severity */
typedef enum
{
   ERRORLOG = 1,
   WARNINGLOG,
   INFOLOG,
   DEBUGLOG,
   DEVELOPERLOG,
   LAST_LOG_SEVERITY
} LogEntrySeverityType; 


/** Definitions of the various Logger subsystem types.
    The 'string' version of these types must be added/changed in
    /platform/logger/LoggerCommon.cpp */
typedef enum
{
   ALL_LOG_SUBSYSTEMS = 0,
   /* Platform Logging Subsystems */
   OPMLOG,
   MSGMGRLOG,
   PROCMGRLOG,
   RSRCMONLOG,
   DATAMGRLOG,
   FAULTMGRLOG,
   THREADLOG,

   /* EMS Logging Subsystems */
   CLIENTAGENTLOG,
   CLICLIENTLOG,

   /* Application Logging Subsystems */

   MAX_LOG_SUBSYSTEM
} LogEntrySubSystemType;


/**
 * Preferred Redundancy Roles
 */
enum PreferredRedundantRole
{
   REDUNDANT_UNKNOWN = 0,
   REDUNDANT_STANDBY,
   REDUNDANT_ACTIVE,
   REDUNDANT_LOADSHARED
};


/** DataManager Definitions for the various field formatting types that
    can be used in database operations (query, insert, delete, etc.) */
typedef enum
{
   BOOLEAN_FIELD_TYPE = 1,
   CHAR_FIELD_TYPE,
   INT8_FIELD_TYPE,
   INT4_FIELD_TYPE,
   INT2_FIELD_TYPE,
   FLOAT4_FIELD_TYPE,
   FLOAT8_FIELD_TYPE,
   VARCHAR_FIELD_TYPE,
   TIMESTAMP_FIELD_TYPE
} DataManagerFieldType;

#endif
