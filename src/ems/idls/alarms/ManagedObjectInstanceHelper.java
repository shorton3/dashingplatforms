package ems.idls.alarms;


/**
* ems/idls/alarms/ManagedObjectInstanceHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from alarms.idl
* Tuesday, January 15, 2015 5:02:23 PM CST
*/


/**
    * Managed Object Instance Identifier Type
    */
abstract public class ManagedObjectInstanceHelper
{
  private static String  _id = "IDL:alarms/ManagedObjectInstance:1.0";

  public static void insert (org.omg.CORBA.Any a, long that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static long extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      __typeCode = org.omg.CORBA.ORB.init ().get_primitive_tc (org.omg.CORBA.TCKind.tk_longlong);
      __typeCode = org.omg.CORBA.ORB.init ().create_alias_tc (ems.idls.alarms.ManagedObjectInstanceHelper.id (), "ManagedObjectInstance", __typeCode);
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static long read (org.omg.CORBA.portable.InputStream istream)
  {
    long value = (long)0;
    value = istream.read_longlong ();
    return value;
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, long value)
  {
    ostream.write_longlong (value);
  }

}
