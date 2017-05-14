/*******************************************************************************
 * 
 * File name:   GuiClient.java
 * Subsystem:   EMS
 * Description: Implements Gui Client for communicating to the EMS via CORBA.
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
import org.omg.CORBA.ORBPackage.*;
import org.omg.PortableServer.*;
import org.omg.CosNaming.*;

//-----------------------------------------------------------------------------
// Component import files, includes elements of our system.
//-----------------------------------------------------------------------------

import ems.idls.session.*;

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
 * GuiClient implements a graphical client interface for communicating to
 * the EMS via CORBA. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

public final class GuiClient
{
   /**
    * Default Constructor
    */
   public GuiClient()
   {

   }//end Default Constructor

   /** Reference to the Gui Widget */
   public GuiFrame guiFrame_ = null;

   /**
    * Initialize the GUI Client Widgets
    */
   public final void initializeGUI()
   {
      guiFrame_ = new GuiFrame();
   }//end initializeGUI


   /** Remote reference to the Naming Service Context */
   private NamingContextExt namingContext_ = null;

   /** Remote reference to the Agent's Session Interface object */
   private Session_I agentSession_ = null;

   /**
    * Initialization Method that JacOrb CORBA initialization
    * @param arguments Command line arguments to the Gui Client are passed directly
    *    to the JacOrb Orb for parsing out Corba specific initialization parameters.
    */
   public final void initializeOrb(String[] arguments)
   {
      try
      {
         // Initialize the Orb
         System.out.println("Initializing the Orb and Activating the POA");
         ORB orb = ORB.init(arguments, null);

         // Initialize the POA
         POA poa = POAHelper.narrow(orb.resolve_initial_references( "RootPOA" ));
         
         // Activate the POA
         poa.the_POAManager().activate();

         ////////////
         // Create our own IDL objects and register with the Orb
         ////////////

         // Create the Gui Client's session Interface Object
         Session_IImpl session_IImpl = new Session_IImpl();

         // Create the object reference for the session interface object
         org.omg.CORBA.Object sessionObjRef = poa.servant_to_reference(session_IImpl);

         // Display the IOR of the session Interface object
         System.out.println("Session Interface Object IOR is " +  orb.object_to_string(sessionObjRef));

         ////////////
         // Resolve and bind to the Naming Service reference
         ////////////

         // Resolve the Naming Service. Here success depends on the parameters passed
         // to the Orb for INS (Interoperable Naming Service) location string (ie. corbaloc)
         System.out.println("Resolving the Naming Service and Narrowing");  
         namingContext_ = NamingContextExtHelper.narrow(orb.resolve_initial_references("NameService"));

         // Bind the session Interface object reference into the Naming Service
         System.out.println("Binding Gui Client Session into the Naming Service with Name ClientSession");
         NameComponent[] sessionName = new NameComponent[1];
         // Name id, kind - this is the same for the CLI -- need to distinguish!!:
         sessionName[0] = new NameComponent("ClientSession", "client"); 
         namingContext_.rebind(sessionName, sessionObjRef);

         ///////////
         // Resolve the Agent (Server Side) Interface Objects from the Naming Service
         ///////////

         // Resolve the Agent Interface
         System.out.println("Resolving and Narrowing Agent Session interface object from the Naming Service");
         NameComponent[] agentName = new NameComponent[1];
         agentName[0] = new NameComponent("AgentSession", "server");
         agentSession_ = Session_IHelper.narrow(namingContext_.resolve(agentName));

         // Store the remote reference to the Agent Session interface object inside our
         // session as the 'AssociatedSession'
         session_IImpl.setAssociatedSession(agentSession_); 
	 guiFrame_.setAgentSession(agentSession_);

         // Start the Orb Event Loop to wait for received requests. This is a blocking
         // call that waits forever
         System.out.println("Running Orb to await notifications and requests");
         orb.run();
      }//end try
      catch (Exception e)
      {
         System.out.println(e);
      }//end catch
   }//end initializeOrb


   /** 
    * String'ized debugging method
    * @return string representation of the contents of this object
    */
   public final String toString()
   {
      return "";
   }//end toString


   /**
    * Gui Client Main Method
    */
   public static void main (String[] aArguments)
   {
      GuiClient guiClient = new GuiClient();

      // Initialize the GUI Client widgets
      guiClient.initializeGUI();

      // Initialize the JacOrb Corba Orb and block waiting for requests
      guiClient.initializeOrb(aArguments);
   }//end main

}//end class GuiClient
