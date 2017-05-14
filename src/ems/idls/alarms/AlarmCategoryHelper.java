package ems.idls.alarms;


/**
* ems/idls/alarms/AlarmCategoryHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from alarms.idl
* Tuesday, January 15, 2015 5:02:23 PM CST
*/


/**
    * Alarm Category Type
    */
abstract public class AlarmCategoryHelper
{
  private static String  _id = "IDL:alarms/AlarmCategory:1.0";

  public static void insert (org.omg.CORBA.Any a, ems.idls.alarms.AlarmCategory that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static ems.idls.alarms.AlarmCategory extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      __typeCode = org.omg.CORBA.ORB.init ().create_enum_tc (ems.idls.alarms.AlarmCategoryHelper.id (), "AlarmCategory", new String[] { "COMMUNICATIONS_ALARM", "QUALITY_OF_SERVICE_ALARM", "PROCESSING_ERROR_ALARM", "EQUIPMENT_ALARM", "ENVIRONMENTAL_ALARM"} );
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static ems.idls.alarms.AlarmCategory read (org.omg.CORBA.portable.InputStream istream)
  {
    return ems.idls.alarms.AlarmCategory.from_int (istream.read_long ());
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, ems.idls.alarms.AlarmCategory value)
  {
    ostream.write_long (value.value ());
  }

}