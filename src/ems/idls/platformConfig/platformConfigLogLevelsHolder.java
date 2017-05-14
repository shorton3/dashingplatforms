package ems.idls.platformConfig;


/**
* ems/idls/platformConfig/platformConfigLogLevelsHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from platformConfig.idl
* Tuesday, January 15, 2015 5:02:24 PM CST
*/


/**
    * Array of all of the subsystem log levels
    */
public final class platformConfigLogLevelsHolder implements org.omg.CORBA.portable.Streamable
{
  public ems.idls.platformConfig.platformConfigLogLevel value[] = null;

  public platformConfigLogLevelsHolder ()
  {
  }

  public platformConfigLogLevelsHolder (ems.idls.platformConfig.platformConfigLogLevel[] initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = ems.idls.platformConfig.platformConfigLogLevelsHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    ems.idls.platformConfig.platformConfigLogLevelsHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return ems.idls.platformConfig.platformConfigLogLevelsHelper.type ();
  }

}