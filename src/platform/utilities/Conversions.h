/******************************************************************************
* 
* File name:   Conversions.h 
* Subsystem:   Platform Services 
* Description: Utility conversion methods.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_UTILITY_CONVERSIONS_H_
#define _PLAT_UTILITY_CONVERSIONS_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

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
 * Conversions contains static conversion utility methods.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class Conversions
{
   public:

      /**
       * Convert an integer to the specified Base.
       * @returns a string representation of the new Base value
       */
      static string integer2Base(unsigned long value, const unsigned int base);

      /** 
       * Convert a binary string to a decimal number.
       * @returns decimal value
       */
      static int binary2Decimal(char* binaryString);

      /** Virtual Destructor */
      virtual ~Conversions();

   protected:

   private:

      /** Constructor */
      Conversions();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      Conversions(const Conversions& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      Conversions& operator= (const Conversions& rhs);

};

#endif
