package ems.idls.alarms;


/**
* ems/idls/alarms/IsFilterEnabledHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from alarms.idl
* Tuesday, January 15, 2015 5:02:23 PM CST
*/


/**
    * Filter Enabled Indicator
    */
abstract public class IsFilterEnabledHelper
{
  private static String  _id = "IDL:alarms/IsFilterEnabled:1.0";

  public static void insert (org.omg.CORBA.Any a, boolean that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static boolean extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      __typeCode = org.omg.CORBA.ORB.init ().get_primitive_tc (org.omg.CORBA.TCKind.tk_boolean);
      __typeCode = org.omg.CORBA.ORB.init ().create_alias_tc (ems.idls.alarms.IsFilterEnabledHelper.id (), "IsFilterEnabled", __typeCode);
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static boolean read (org.omg.CORBA.portable.InputStream istream)
  {
    boolean value = false;
    value = istream.read_boolean ();
    return value;
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, boolean value)
  {
    ostream.write_boolean (value);
  }

}
