/******************************************************************************
* 
* File name:   CliMenuBaseActions.h 
* Subsystem:   EMS
* Description: Abstract Base class for Menu Item Handlers
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _EMS_CLI_MENU_BASE_ACTIONS_H_
#define _EMS_CLI_MENU_BASE_ACTIONS_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

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
 * CliMenuBaseActions is an Abstract Base class for Menu Item Handlers. 
 * <p>
 * CliMenuBaseActions is used by the CliMenuController for generically
 * calling the Menu Item handlers.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class CliMenuBaseActions
{
   public:

      /** Ptr to member function returning void */
      typedef void (CliMenuBaseActions::*CLIVOIDFUNCPTR) (...);

      /** Ptr to member function returning int */
      typedef int (CliMenuBaseActions::*CLIFUNCPTR) (...);

      /** Constructor */
      CliMenuBaseActions();

      /** Virtual Destructor */
      virtual ~CliMenuBaseActions();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      CliMenuBaseActions(const CliMenuBaseActions& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      CliMenuBaseActions& operator= (const CliMenuBaseActions& rhs);

};

#endif
