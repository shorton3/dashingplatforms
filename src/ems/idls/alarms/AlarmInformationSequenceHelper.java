package ems.idls.alarms;


/**
* ems/idls/alarms/AlarmInformationSequenceHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from alarms.idl
* Tuesday, January 15, 2015 5:02:23 PM CST
*/


/**
    * Sequence(vector) of Alarm Information structures
    */
abstract public class AlarmInformationSequenceHelper
{
  private static String  _id = "IDL:alarms/AlarmInformationSequence:1.0";

  public static void insert (org.omg.CORBA.Any a, ems.idls.alarms.AlarmInformation[] that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static ems.idls.alarms.AlarmInformation[] extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      __typeCode = ems.idls.alarms.AlarmInformationHelper.type ();
      __typeCode = org.omg.CORBA.ORB.init ().create_sequence_tc (0, __typeCode);
      __typeCode = org.omg.CORBA.ORB.init ().create_alias_tc (ems.idls.alarms.AlarmInformationSequenceHelper.id (), "AlarmInformationSequence", __typeCode);
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static ems.idls.alarms.AlarmInformation[] read (org.omg.CORBA.portable.InputStream istream)
  {
    ems.idls.alarms.AlarmInformation value[] = null;
    int _len0 = istream.read_long ();
    value = new ems.idls.alarms.AlarmInformation[_len0];
    for (int _o1 = 0;_o1 < value.length; ++_o1)
      value[_o1] = ems.idls.alarms.AlarmInformationHelper.read (istream);
    return value;
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, ems.idls.alarms.AlarmInformation[] value)
  {
    ostream.write_long (value.length);
    for (int _i0 = 0;_i0 < value.length; ++_i0)
      ems.idls.alarms.AlarmInformationHelper.write (ostream, value[_i0]);
  }

}
