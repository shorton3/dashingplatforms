package ems.idls.alarms;


/**
* ems/idls/alarms/SeverityHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from alarms.idl
* Tuesday, January 15, 2015 5:02:23 PM CST
*/


/**
    * Alarm Severity Type (User Severity gets reported)
    */
abstract public class SeverityHelper
{
  private static String  _id = "IDL:alarms/Severity:1.0";

  public static void insert (org.omg.CORBA.Any a, ems.idls.alarms.Severity that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static ems.idls.alarms.Severity extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      __typeCode = org.omg.CORBA.ORB.init ().create_enum_tc (ems.idls.alarms.SeverityHelper.id (), "Severity", new String[] { "CRITICAL", "MAJOR", "MINOR", "WARNING", "CLEAR"} );
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static ems.idls.alarms.Severity read (org.omg.CORBA.portable.InputStream istream)
  {
    return ems.idls.alarms.Severity.from_int (istream.read_long ());
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, ems.idls.alarms.Severity value)
  {
    ostream.write_long (value.value ());
  }

}
