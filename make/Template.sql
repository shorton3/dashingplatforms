--******************************************************************************
--
-- File name:   Template.sql
-- Subsystem:   SQL Database Schema
-- Description: 
--
-- Name                 Date       Release
-- -------------------- ---------- ---------------------------------------------
-- Your Name            01/01/2014 Initial release
--
--
--*****************************************************************************/

-- Create the schema
create table template(
  field1          int4,
  field2          int4,
primary key (field1));

grant all on template to userid;

-- Populate default values
insert into template (field1, field2) values ('someValue1', 'someValue2');
