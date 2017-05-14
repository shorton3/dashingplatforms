/******************************************************************************
* 
* File name:   FixCompilerWarning.h 
* Subsystem:   Platform Services 
* Description: This is a class created strictly to remove a warning that the
*              compiler is generating.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_FIX_COMPILER_WARNING_H_
#define _PLAT_FIX_COMPILER_WARNING_H_

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
 * FixCompilerWarning is a class created strictly to remove a warning that
 * the compiler is generating. 
 * <p>
 * If the compiler finds a class that does not have a public constructor
 * and does not have any friend classes, it will generate a warning.
 * This situation occurs fairly often in our code. To remove the warning
 * include this file and add:
 *      friend class RzFixCompilerWarning;
 * to the class definition that is generating the warning.  The
 * FixCompilerWarning class has no implementation and no methods. But,
 * because the compiler sees it as a friend of the class with a private
 * constructor, it will not generate the warning messsage.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class FixCompilerWarning
{
   public:

      /** Constructor */
      FixCompilerWarning();

      /** Virtual Destructor */
      virtual ~FixCompilerWarning();

      /** Dummy method */
      virtual void neverImplementThis(void) = 0;

   protected:

   private:

};

#endif
