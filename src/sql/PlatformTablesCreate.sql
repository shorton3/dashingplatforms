--******************************************************************************
--
-- File name:   PlatformTablesCreate.sql
-- Subsystem:   SQL Database Schema
-- Description: Create the platform table schema
--
-- Name                 Date       Release
-- -------------------- ---------- ---------------------------------------------
-- Stephen Horton       01/01/2014 Initial release
--
--
--*****************************************************************************/


-- Create a schema called 'platform'. This schema will contain all platform related
-- database tables. Application tables should created in a separate schema. The
-- reason for this is that DATA SEPARATION requirements may indicate that OA&M
-- management data and customer bearing data be kept separate and that database
-- access should happen on physically separate networks (Layer1 data separation)
create schema platform;

-- NEID is the top level primary index. Each Network Element is represented by an
--   NEID. NEID consists of 9 characters: AAABBBCCC, where AAA is the customer Id,
--   BBB is the cluster identifier, and CCC is the instance within the cluster.
-- PEERIPAddress is the IP Address of the redundant mate for this network element.
-- DbUserid and DbPassword will need to be stored in encrypted form (later).
create table platform.NetworkElements (
  NEID            character(9) not null,
  IPAddress       varchar(15) not null,
  PeerIPAddress   varchar(15),
  DataSourceName  varchar(30) not null,
  DbUserid        varchar(30) not null,
  DbPassword      varchar(30) not null,
primary key (NEID));


-- Additional information for each Network Element
create table platform.NetworkElementInfo (
  NEID            character(9) not null,
  NEName          varchar(15),
  Location        varchar(30),
  City            varchar(15),
  ZipCode         varchar(15),
  RackNumber      int2 not null,
  ShelfNumber     int2 not null,
  SlotNumber      int2 not null,
foreign key (NEID) references platform.NetworkElements(NEID));


-- Configured tracelog levels for each network element
create table platform.LogLevels (
  NEID            character(9) not null,
  SubSystem       int2 not null,
  LogLevel        int2 not null,
primary key (NEID, SubSystem),
foreign key (NEID) references platform.NetworkElements(NEID));


-- Configured Alarms Inventory for the entire platform.
-- Severity represents the 'default' severity used by the system. We are required
--   to maintain this for redundancy/failover analysis purposes.
-- UserSeverity is the perceived severity of the user, which the user can re-assign.
--   (This is what should be reported to the OSS/NOC as well as EMS client, and 
--   it should be initially datafilled the same as 'Severity')
create table platform.AlarmsInventory (
  AlarmCode       int2 not null,
  Category        int2 not null,
  HighWaterMark   int4,
  LowWaterMark    int4,
  Severity        int2 not null,
  UserSeverity    int2 not null,
  AutoClearing    boolean not null,
  FilterEnabled   boolean not null,
  Description     varchar(500) not null,
  Resolution      varchar(500) not null,
primary key (AlarmCode));


-- Configured Event Reports Inventory for the entire platform.
create table platform.EventReportsInventory (
  EventCode       int2 not null,
  FilterEnabled   boolean not null,
  Description     varchar(500) not null,
primary key (EventCode));


-- Outstanding Alarms table. Each network element maintains a local outstanding
-- alarms list/table, and the EMS maintains a system wide outstanding alarms list.
-- This table is for the EMS consolidated alarms, and provides a historical record
-- of Alarms, when they got cleared, and the acknowledgement text. We need a 
-- strategy for how this table gets cleaned up.
--
-- OriginatedByNEID - will be different from ApplicableToNEID if this alarm was
--   raised on behalf of another node (ie. another node sees ApplicableTo node go down)
-- ApplicableToNEID - the node the Alarm applies to (also indicates which node's
--   LocalAlarmsOutstanding table the alarms exists in)
-- (In a NON-proxy(on behalf) condition, OriginatedByNEID == ApplicableToNEID)
-- IsCleared indicates that the alarm has been cleared and is no longer outstanding;
--   however, we do not want to delete the alarm for historical purposes of displaying
--   the acknowledgement text as well as date/time.
-- Since we are keeping Cleared Alarms for historical purposes (use of IsCleared), then
--   TimeStamp needs to be part of the key to prevent a violation when an Alarm is
--   raised that was previously cleared (and is still in the table).
create table platform.AlarmsOutstanding (
  OriginatedByNEID  character(9) not null,
  ApplicableToNEID  character(9) not null,
  AlarmCode       int2 not null,
  ManagedObject   int2 not null,
  ManagedObjectInstance  int4 not null,
  Pid             int4 not null,
  TimeStamp       timestamp with time zone not null,
  Acknowledgement varchar(500),
  IsCleared       boolean not null, 
primary key (OriginatedByNEID, ApplicableToNEID, AlarmCode, ManagedObject, ManagedObjectInstance, Pid, TimeStamp),
foreign key (OriginatedByNEID) references platform.NetworkElements(NEID),
foreign key (ApplicableToNEID) references platform.NetworkElements(NEID),
foreign key (AlarmCode) references platform.AlarmsInventory(AlarmCode));


-- Outstanding Alarms table. Each network element maintains a local outstanding
-- alarms list/table, and the EMS maintains a system wide outstanding alarms list.
-- This table is for the local node's outstanding alarms.
create table platform.LocalAlarmsOutstanding (
  NEID            character(9) not null,
  AlarmCode       int2 not null,
  ManagedObject   int2 not null,
  ManagedObjectInstance  int4 not null,
  Pid             int4 not null,
  TimeStamp       timestamp with time zone not null,
  Acknowledgement varchar(500),
primary key (NEID, AlarmCode, ManagedObject, ManagedObjectInstance, Pid),
foreign key (NEID) references platform.NetworkElements(NEID),
foreign key (AlarmCode) references platform.AlarmsInventory(AlarmCode));


