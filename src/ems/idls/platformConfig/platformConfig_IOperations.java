package ems.idls.platformConfig;


/**
* ems/idls/platformConfig/platformConfig_IOperations.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from platformConfig.idl
* Tuesday, January 15, 2015 5:02:24 PM CST
*/


/**
    * Command/Method interface
    */
public interface platformConfig_IOperations 
{

  /**
         * Retrieve all of the subsystem log levels
         */
  void getLogLevels (ems.idls.platformConfig.platformConfigLogLevelsHolder logLevels);

  /**
         * Set a specified log level 
         */
  void setLogLevel (ems.idls.platformConfig.platformConfigLogLevel logLevel);

  /**
         * Set all of the log levels
         */
  void setLogLevels (ems.idls.platformConfig.platformConfigLogLevel[] logLevels);
} // interface platformConfig_IOperations