/******************************************************************************
*
* File name:   OPMTest.h
* Subsystem:   Platform Services
* Description: Implements the unit test code to exercise the OPM.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_OPM_TEST_H_
#define _PLAT_OPM_TEST_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/opm/OPMBase.h"

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
 * Test class header file inherits from OPMBase
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class OPMTest : public OPMBase
{
   public:
      
      /**
       * Method will be called when the pool is created with these objects.
       * This method bootstraps the creation of each object. THIS OVERRIDES
       * the initialize method in OPMBASE (can't be both virtual and static)
       * @param initializer - Initializer integer or pointer to initialization
       *      data needed by the object - may be 0 if unused.
       * @returns Pointer to the new OPMBase object.
       */
      static OPMBase* initialize(int initializer);

      /**
       * Pure Virtual method will be called before releasing the object 
       * back into the pool.
       */
      void clean();

      /** Test method */
      void doSomething();

   protected:

   private:

};

#endif

