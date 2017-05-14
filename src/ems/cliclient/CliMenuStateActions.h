/******************************************************************************
* 
* File name:   CliMenuStateActions.h 
* Subsystem:   EMS
* Description: Static handler methods for the State Transition Command
*              Menu Items.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _EMS_CLI_MENU_STATE_ACTIONS_H_
#define _EMS_CLI_MENU_STATE_ACTIONS_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "sessionI.h"
#include "CliMenuBaseActions.h"

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
 * CliMenuStateActions contains static handler methods for the State Transition
 * Command Menu Items. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class CliMenuStateActions : public CliMenuBaseActions
{
   public:

      /** Constructor */
      CliMenuStateActions();

      /** Virtual Destructor */
      virtual ~CliMenuStateActions();

      /** Initialization Method */ 
      void initialize(session::Session_I_ptr session_I_ptr);

      /** Unlock State Transition Command */
      void sendUnlock();

      /** Lock State Transition Command */
      void sendLock();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      CliMenuStateActions(const CliMenuStateActions& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      CliMenuStateActions& operator= (const CliMenuStateActions& rhs);

      /** Pointer to the Session interface object. Here we use a pointer since
          we're passing in the reference from the main class that handles the Corba stuff */
      session::Session_I_ptr session_I_ptr_;

};

#endif
