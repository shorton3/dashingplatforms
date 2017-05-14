package ems.idls.session;

/**
* ems/idls/session/Session_IHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from session.idl
* Tuesday, January 15, 2015 5:02:24 PM CST
*/


/**
   * <p> The Session_I interface provides capabilities to manage the
   * client-server connection.
   * Its main purpose is to enable either a client or server to detect the 
   * loss of communication with the associated party.</p>
   *
   * <p>For a single communication session between an NMS and an EMS, there are
   * two Session_I objects.  One is maintained on the NMS; the other one is maintained
   * on the EMS.  The Session_I object maintained on the EMS is actually an EmsSession_I, while
   * the Session_I object maintained on the NMS is actually an NmsSession_I
   * (both inherit from Session_I).</p>
   *
   * <p>Each Session_I object is responsible to "ping" the other Session_I object
   * periodically to detect communication failures.  Exactly when this is done is up to
   * the implementation.</p>
   *
   * <p>When a Session_I object detects a communication failure, or when the endSession
   * operation is called on it, all resources allocated with that communication session
   * must be freed and the Session_I object must be deleted.</p>
   **/
public final class Session_IHolder implements org.omg.CORBA.portable.Streamable
{
  public ems.idls.session.Session_I value = null;

  public Session_IHolder ()
  {
  }

  public Session_IHolder (ems.idls.session.Session_I initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = ems.idls.session.Session_IHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    ems.idls.session.Session_IHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return ems.idls.session.Session_IHelper.type ();
  }

}
