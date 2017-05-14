/******************************************************************************
* 
* File name:   ObjectBase.h 
* Subsystem:   Platform Services 
* Description: Simple base class for identifying objects as OPM pool-able
*              or not (that is, whether or not they inherit from OPMBase).
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_OBJECT_BASE_H_
#define _PLAT_OBJECT_BASE_H_

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
 * Simple base class for identifying objects as OPM poolable or not, that is,
 * whether or not they inherit from OPMBase.
 * <p>
 * Using this class the OPM and MsgMgr frameworks can distinguish between
 * objects and messages that need to be instantiated and then deleted versus
 * objects and messages that should be retrieved from a message pool and 
 * checked back in. Application developers should NOT derive from this class.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ObjectBase
{
   public:

      /** Constructor */
      ObjectBase();

      /** Virtual Destructor */
      virtual ~ObjectBase();

      /** Set whether or not this object is OPM poolable, that is, whether
          or not the object inherits from OPMBase. */
      void setPoolable(bool isPoolable);

   protected:

      /** Return whether or not this object is OPM poolable (inherits from OPMBase) */
      bool isPoolable();

   private:

      /** Flag for determining if the object is OPM poolable -- ie. that
          it extends from OPMBase */
      bool isPoolable_;
};

#endif
