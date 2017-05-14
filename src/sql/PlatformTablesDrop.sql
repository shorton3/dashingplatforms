--******************************************************************************
--
-- File name:   PlatformTablesDrop.sql
-- Subsystem:   SQL Database Schema
-- Description: Drop the platform table schema
--
-- Name                 Date       Release
-- -------------------- ---------- ---------------------------------------------
-- Stephen Horton       01/01/2014 Initial release
--
--
--*****************************************************************************/


-- Drop all platform tables in the reverse order in which they were created
drop table platform.AlarmsOutstanding;
drop table platform.LocalAlarmsOutstanding;
drop table platform.AlarmsInventory;
drop table platform.EventReportsInventory;
drop table platform.LogLevels;
drop table platform.NetworkElementInfo;
drop table platform.NetworkElements;

-- Drop the platform schema after all database objects have been dropped
drop schema platform;

