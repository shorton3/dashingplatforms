package ems.idls.alarms;


/**
* ems/idls/alarms/AlarmNotificationSequenceHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from alarms.idl
* Tuesday, January 15, 2015 5:02:23 PM CST
*/


/**
    * Sequence(Vector) of AlarmNotification structures
    */
public final class AlarmNotificationSequenceHolder implements org.omg.CORBA.portable.Streamable
{
  public ems.idls.alarms.AlarmNotification value[] = null;

  public AlarmNotificationSequenceHolder ()
  {
  }

  public AlarmNotificationSequenceHolder (ems.idls.alarms.AlarmNotification[] initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = ems.idls.alarms.AlarmNotificationSequenceHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    ems.idls.alarms.AlarmNotificationSequenceHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return ems.idls.alarms.AlarmNotificationSequenceHelper.type ();
  }

}
