package ems.idls.platformConfig;


/**
* ems/idls/platformConfig/platformConfigLogLevelHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from platformConfig.idl
* Tuesday, January 15, 2015 5:02:24 PM CST
*/

abstract public class platformConfigLogLevelHelper
{
  private static String  _id = "IDL:platformConfig/platformConfigLogLevel/platformConfigLogLevel:1.0";

  public static void insert (org.omg.CORBA.Any a, ems.idls.platformConfig.platformConfigLogLevel that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static ems.idls.platformConfig.platformConfigLogLevel extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  private static boolean __active = false;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      synchronized (org.omg.CORBA.TypeCode.class)
      {
        if (__typeCode == null)
        {
          if (__active)
          {
            return org.omg.CORBA.ORB.init().create_recursive_tc ( _id );
          }
          __active = true;
          org.omg.CORBA.StructMember[] _members0 = new org.omg.CORBA.StructMember [3];
          org.omg.CORBA.TypeCode _tcOf_members0 = null;
          _tcOf_members0 = org.omg.CORBA.ORB.init ().get_primitive_tc (org.omg.CORBA.TCKind.tk_short);
          _members0[0] = new org.omg.CORBA.StructMember (
            "subsystemEnum",
            _tcOf_members0,
            null);
          _tcOf_members0 = org.omg.CORBA.ORB.init ().create_string_tc (0);
          _members0[1] = new org.omg.CORBA.StructMember (
            "subsystemName",
            _tcOf_members0,
            null);
          _tcOf_members0 = org.omg.CORBA.ORB.init ().get_primitive_tc (org.omg.CORBA.TCKind.tk_short);
          _members0[2] = new org.omg.CORBA.StructMember (
            "severityLevel",
            _tcOf_members0,
            null);
          __typeCode = org.omg.CORBA.ORB.init ().create_struct_tc (ems.idls.platformConfig.platformConfigLogLevelHelper.id (), "platformConfigLogLevel", _members0);
          __active = false;
        }
      }
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static ems.idls.platformConfig.platformConfigLogLevel read (org.omg.CORBA.portable.InputStream istream)
  {
    ems.idls.platformConfig.platformConfigLogLevel value = new ems.idls.platformConfig.platformConfigLogLevel ();
    value.subsystemEnum = istream.read_short ();
    value.subsystemName = istream.read_string ();
    value.severityLevel = istream.read_short ();
    return value;
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, ems.idls.platformConfig.platformConfigLogLevel value)
  {
    ostream.write_short (value.subsystemEnum);
    ostream.write_string (value.subsystemName);
    ostream.write_short (value.severityLevel);
  }

}
