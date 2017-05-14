/******************************************************************************
* 
* File name:   TemplateApp.h 
* Subsystem:   Platform Services 
* Description: <Application description goes here>
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Your Name            01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_TEMPLATE_APPLICATION_H_
#define _PLAT_TEMPLATE_APPLICATION_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Reactor.h>
#include <ace/Signal.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

// For C++ class declarations, we have one (and only one) of these access 
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors/destructors first)
// 5) instance data
//

/**
 * <Short Description goes here> The TemplateApp does blah.
 * <p>
 * <Long Description details go here> TemplateApp does details about blah, blah.
 * <p>
 * TemplateApp perhaps some more details about blah.
 * <p>
 * TemplateApp accepts the following startup options:
 * -l Local Logger Mode only (do not enqueue logs to shared memory)
 * -y <argument of y> About what y does...
 * -z About what z does...
 * <p>
 * $Author: Your Name$
 * $Revision: 1$
 */

class TemplateApp
{
   public:

      /** Constructor */
      TemplateApp();

      /** Virtual Destructor */
      virtual ~TemplateApp();

      /** Start the mailbox processing loop */
      void processMailbox();

      /** Catch the shutdown signal and begin the graceful shutdown */
      static void catchShutdownSignal();

      /** Perform TemplateApp initialization */
      int initialize(int argc, ACE_TCHAR *argv[]);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /** Retrieve the static singleton instance of the TemplateApp */
      static TemplateApp* getInstance();

      /** Start the reactor processing thread for signal handling, etc. */
      static void startReactor();

      /** Gracefully shutdown the TemplateApp and all of its threads */
      void shutdown();

      /**
       * Called in the constructor to setup the mailbox, activate and
       * to setup handlers.
       * @param distributedAddress Mailbox Address of the TemplateApp DistributedMailbox
       * @returns true if successful
       */
      bool setupMailbox(const MailboxAddress& distributedAddress);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      TemplateApp(const TemplateApp& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      TemplateApp& operator= (const TemplateApp& rhs);

      /** Static singleton instance */
      static TemplateApp* templateApp_;

      /** TemplateApp Mailbox */
      MailboxOwnerHandle* templateAppMailbox_;

      /** Message Handler List for storing Functors for Mailbox handlers */
      MessageHandlerList* messageHandlerList_;

      /** Signal Adapter for registering signal handlers */
      ACE_Sig_Adapter* signalAdapter_;

      /** Signal Set for registering signal handlers */
      ACE_Sig_Set signalSet_;

      /** ACE_Select_Reactor used for signal handling */
      static ACE_Reactor* selectReactor_;
};

#endif
