/******************************************************************************
*
* File name:   MailboxNames.h
* Subsystem:   Platform Services
* Description: Contains definitions for all the well known Mailbox Names in the
*              system and their associated ports if they are distributed
*              mailboxes.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/
                                                                                                                          
#ifndef _PLAT_MAILBOX_NAMES_H_
#define _PLAT_MAILBOX_NAMES_H_

#define LOCAL_IP_ADDRESS               "127.0.0.1"

#define UNKNOWN_MAILBOX_NAME           "UNKNOWN"

// Platform Mailbox Names
#define LOGGER_MAILBOX_NAME            "Logger"
#define DISCOVERY_MANAGER_MAILBOX_NAME "DiscoveryManager"
#define DISCOVERY_MANAGER_IP_ADDRESS   "224.9.9.1"
#define DISCOVERY_MANAGER_MAILBOX_PORT 12775
#define FAULT_MANAGER_MAILBOX_NAME     "FaultManager"
#define FAULT_MANAGER_MAILBOX_PORT     12776
#define PROCMGR_MAILBOX_NAME           "ProcessManager"                 
#define PROCMGR_MAILBOX_PORT           12777 
#define RESOURCE_MONITOR_MAILBOX_NAME  "ResourceMonitor"
#define RESOURCE_MONITOR_MAILBOX_PORT  12778

// EMS Mailbox Names
#define CLIENT_AGENT_MAILBOX_NAME      "ClientAgent"
#define CLIENT_AGENT_MAILBOX_PORT      12877


#endif
