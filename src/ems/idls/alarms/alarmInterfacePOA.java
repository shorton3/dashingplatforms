package ems.idls.alarms;


/**
* ems/idls/alarms/alarmInterfacePOA.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from alarms.idl
* Tuesday, January 15, 2015 5:02:23 PM CST
*/


/**
    * Command/Method interface
    */
public abstract class alarmInterfacePOA extends org.omg.PortableServer.Servant
 implements ems.idls.alarms.alarmInterfaceOperations, org.omg.CORBA.portable.InvokeHandler
{

  // Constructors

  private static java.util.Hashtable _methods = new java.util.Hashtable ();
  static
  {
    _methods.put ("getOutstandingAlarms", new java.lang.Integer (0));
    _methods.put ("getAlarmInventory", new java.lang.Integer (1));
  }

  public org.omg.CORBA.portable.OutputStream _invoke (String $method,
                                org.omg.CORBA.portable.InputStream in,
                                org.omg.CORBA.portable.ResponseHandler $rh)
  {
    org.omg.CORBA.portable.OutputStream out = null;
    java.lang.Integer __method = (java.lang.Integer)_methods.get ($method);
    if (__method == null)
      throw new org.omg.CORBA.BAD_OPERATION (0, org.omg.CORBA.CompletionStatus.COMPLETED_MAYBE);

    switch (__method.intValue ())
    {

  /**
         * Retrieve the list of outstanding Alarms from the EMS. This is used when
         * the client first establishes connection and when the client wishes to 
         * resynchronize its outstanding alarms list.
         */
       case 0:  // alarms/alarmInterface/getOutstandingAlarms
       {
         ems.idls.alarms.AlarmNotificationSequenceHolder outstandingAlarms = new ems.idls.alarms.AlarmNotificationSequenceHolder ();
         this.getOutstandingAlarms (outstandingAlarms);
         out = $rh.createReply();
         ems.idls.alarms.AlarmNotificationSequenceHelper.write (out, outstandingAlarms.value);
         break;
       }


  /**
         * Retrieve the list of all possible Alarms in the system. This list includes
         * all Alarm descriptors and attributes and should be used by the client
         * entity for displaying outstanding alarms.
         */
       case 1:  // alarms/alarmInterface/getAlarmInventory
       {
         ems.idls.alarms.AlarmInformationSequenceHolder alarmInventory = new ems.idls.alarms.AlarmInformationSequenceHolder ();
         this.getAlarmInventory (alarmInventory);
         out = $rh.createReply();
         ems.idls.alarms.AlarmInformationSequenceHelper.write (out, alarmInventory.value);
         break;
       }

       default:
         throw new org.omg.CORBA.BAD_OPERATION (0, org.omg.CORBA.CompletionStatus.COMPLETED_MAYBE);
    }

    return out;
  } // _invoke

  // Type-specific CORBA::Object operations
  private static String[] __ids = {
    "IDL:alarms/alarmInterface:1.0"};

  public String[] _all_interfaces (org.omg.PortableServer.POA poa, byte[] objectId)
  {
    return (String[])__ids.clone ();
  }

  public alarmInterface _this() 
  {
    return alarmInterfaceHelper.narrow(
    super._this_object());
  }

  public alarmInterface _this(org.omg.CORBA.ORB orb) 
  {
    return alarmInterfaceHelper.narrow(
    super._this_object(orb));
  }


} // class alarmInterfacePOA
