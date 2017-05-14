/*******************************************************************************
 * 
 * File name:   Session_IImpl.java
 * Subsystem:   EMS
 * Description: Extends from the Session_IPOA skeleton file (server implementation)
 *              that was created from running the idlj Java IDL compiler on the
 *              session.IDL file. Here we add the Gui Client's implementation
 *              for the session interface.
 * 
 * Name                 Date       Release 
 * -------------------- ---------- ---------------------------------------------
 * Stephen Horton       01/01/2014 Initial release 
 * 
 *
 ******************************************************************************/
package ems.guiclient;

//-----------------------------------------------------------------------------
// System import files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

import org.omg.CORBA.*;

//-----------------------------------------------------------------------------
// Component import files, includes elements of our system.
//-----------------------------------------------------------------------------

import ems.idls.session.Session_IPOA;
import ems.idls.session.Session_I;

// For Java class declarations, we have one (and only one) of these access 
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors first)
// 5) instance data
//

/**
 * Session_IImpl extends from the Session_IPOA skeleton file (server implementation)
 * that was created from running the idlj Java IDL compiler on the
 * session.IDL file. 
 * <p>
 * Here we add the Gui Client's implementation for the session interface.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

public final class Session_IImpl extends Session_IPOA
{
   /**
    * Default Constructor
    */
   public Session_IImpl()
   {

   }//end Default Constructor

 
   /** Reference to the associated session */
   public ems.idls.session.Session_I associatedSession_ = null;


   /**
    * Return a reference to the remote session for determining communication validity
    */
   public ems.idls.session.Session_I associatedSession ()
   {
      return associatedSession_;
   }//end associatedSession


   /**
    * Respond to a remotely invoked Ping method
    */
   public void ping()
   {

   }//end ping


   /**
    * Respond to a remotely invoked End Session request
    */
   public void endSession()
   {

   }//end endSession


   /**
    * Store the associated session
    */
   public void setAssociatedSession(ems.idls.session.Session_I associatedSession)
   {
      associatedSession_ = associatedSession;
   }//end setAssociatedSession


   /** 
    * String'ized debugging method
    * @return string representation of the contents of this object
    */
   public final String toString()
   {
      return "";
   }//end toString


}//end class Session_IImpl
