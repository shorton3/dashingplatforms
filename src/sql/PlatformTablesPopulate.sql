--******************************************************************************
--
-- File name:   PlatformTablesPopulate.sql
-- Subsystem:   SQL Database Schema
-- Description: 
--
-- Name                 Date       Release
-- -------------------- ---------- ---------------------------------------------
-- Stephen Horton       01/01/2014 Initial release
--
--
--*****************************************************************************/

-- Populate default values

-------------------------------------------------------------------------------
-- For NetworkElements (platform.NetworkElements)
-------------------------------------------------------------------------------
-- Create a test network element (This table will be typically populated via the EMS)
insert into platform.NetworkElements (NEID, IPAddress, PeerIPAddress, DataSourceName, DBUserid, DbPassword)
            values ('100000001', '127.0.0.1', '127.0.0.2', 'platform', 'platform', 'platform');

-------------------------------------------------------------------------------
-- For NetworkElementInfo (platform.NetworkElementInfo)
-------------------------------------------------------------------------------
-- Create information elements for the test NE above (typically populated via the EMS)
insert into platform.NetworkElementInfo (NEID, NEName, Location, City, ZipCode, RackNumber, ShelfNumber, SlotNumber)
            values ('100000001', 'TestNE', 'Home', 'Dallas', '75407', 2, 3, 4);

-------------------------------------------------------------------------------
-- For Platform Logger (platform.LogLevels) - from platform/common/Defines.h:
--      ERRORLOG = 1, WARNINGLOG = 2, INFOLOG = 3, DEBUGLOG = 4, DEVELOPERLOG = 5
-- Here, create the log levels for our test NE (typically populated via the EMS)
-------------------------------------------------------------------------------
-- OPM Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 1, 5);
-- MSGMGR Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 2, 5);
-- PROCMGR Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 3, 5);
-- RSRCMON Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 4, 5);
-- DATAMGR Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 5, 5);
-- FAULTMGR Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 6, 5);
-- THREADMGR Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 7, 5);
-- CLIENTAGENT Subsystem, Developer Level
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 8, 5);
-- CLICLIENT Subsystem, Developer Level (NOTE: This is just FILLER, since CLI doesn't use
-- the log levels set in the database
insert into platform.LogLevels (NEID, SubSystem, LogLevel) values ('100000001', 9, 5);


-------------------------------------------------------------------------------
-- For the Alarm Inventory (platform.AlarmsInventory) - must match platform/common/AlarmsEvents.h
-- NOTE: You must perform the hex to decimal conversion here (only use the decimal values for the code)
--    Category: Communication = 0, QoS = 2, Processing Error = 3, Equipment = 4, Environmental = 5
--    Severity: Indeterminate = 0, Critical = 1, Major = 2, Minor = 3, Warning = 4, Clear = 5, Informational = 6
-- This table must be populated by the EMS / Database installation script as this
-- is a system wide table (and not NE specific); however, the High and Low Water Mark
-- fields, User Severity, FilterEnabled, and Resolution fields should be user-configurable via EMS.
-- Initially datafill UserSeverity the same as Severity.
-------------------------------------------------------------------------------
-- OS RESOURCE DISKUSAGE
insert into platform.AlarmsInventory (AlarmCode, Category, HighWaterMark, LowWaterMark, Severity,
            UserSeverity, AutoClearing, FilterEnabled, Description, Resolution)
            values (1, 3, 80, 0, 2, 2, true, false,
            'Disk usage of a partition has exceeded 80%',
            'Check log file and performance stats directories for logs and core files that can be deleted');
-- OS RESOURCE CPUUSAGE
insert into platform.AlarmsInventory (AlarmCode, Category, HighWaterMark, LowWaterMark, Severity,
            UserSeverity, AutoClearing, FilterEnabled, Description, Resolution)
            values (2, 3, 80, 0, 2, 2, true, false,
            'CPU usage has exceeded 80%',
            'Check logs for software errors or consider scaling this network element');
-- OS RESOURCE MEMORYUSAGE
insert into platform.AlarmsInventory (AlarmCode, Category, HighWaterMark, LowWaterMark, Severity,
            UserSeverity, AutoClearing, FilterEnabled, Description, Resolution)
            values (3, 3, 80, 0, 2, 2, true, false,
            'Memory usage has exceeded 80%',
            'Check logs for software errors or consider scaling this network element');
-- DATA MANAGER CONNECTIONFAILED
insert into platform.AlarmsInventory (AlarmCode, Category, HighWaterMark, LowWaterMark, Severity,
            UserSeverity, AutoClearing, FilterEnabled, Description, Resolution)
            values (1025, 0, 0, 0, 2, 2, true, false,
            'Database connection failure has occurred',
            'Check network connectivity to remote database or check local/remote database health');


-------------------------------------------------------------------------------
-- For the Event Reports Inventory (platform.EventReportsInventory) - must match platform/common/AlarmsEvents.h
-- NOTE: You must perform the hex to decimal conversion here (only use the decimal values for the code)
-- This table must be populated by the EMS / Database installation script as this
-- is a system wide table (and not NE specific); however, the FilterEnabled field
-- should be user-configurable via EMS.
-------------------------------------------------------------------------------
-- OS RESOURCE DISKCHECK
insert into platform.EventReportsInventory (EventCode, FilterEnabled, Description)
            values (1, false, 'Performing Disk usage check for all partitions');
-- OS RESOURCE MEMORYCHECK
insert into platform.EventReportsInventory (EventCode, FilterEnabled, Description)
            values (2, false, 'Performing Memory usage check');
-- OS RESOURCE CPUCHECK
insert into platform.EventReportsInventory (EventCode, FilterEnabled, Description)
            values (3, false, 'Performing CPU usage check');
-- DATA MANAGER CONNECTIONCHECK
insert into platform.EventReportsInventory (EventCode, FilterEnabled, Description)
            values (1025, false, 'Performing database connection health check');
