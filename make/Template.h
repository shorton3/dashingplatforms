/******************************************************************************
* 
* File name:   Template.h 
* Subsystem:   Platform Services 
* Description: !!!! Put your description here !!!!
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Your Name            01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_TEMPLATE_FILE_H_
#define _PLAT_TEMPLATE_FILE_H_

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
 * Template short description here. 
 * <p>
 * Template long description here.
 * blah, blah, blah
 * <p>
 * $Author: Your Name$
 * $Revision: 1$
 */

class Template
{
   public:

      /** Constructor */
      Template();

      /** Virtual Destructor */
      virtual ~Template();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      Template(const Template& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      Template& operator= (const Template& rhs);

};

#endif
