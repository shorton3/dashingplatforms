/******************************************************************************
* 
* File name:   OPMBase.h 
* Subsystem:   Platform Services 
* Description: Implements the Object Pool Manager Base class for all OPM 
*              managed objects.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_OPM_BASE_H_
#define _PLAT_OPM_BASE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>
#include <cstdio>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ObjectBase.h"

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
 * Interface for OPM based objects
 * <p>
 * OPMBase interface contains callback methods for initializing and cleaning
 * an Object when getting it and releasing it back into the OPM Object Pool.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

#define UNKNOWN_POOLID -1

class OPMBase : public ObjectBase
{
   public:

      /** Constructor */
      OPMBase();

      /** Virtual Destructor */
      virtual ~OPMBase();

      /** Return whether or not there is a next node in the Linked List */
      bool isNextNull();

      /** Return whether or not there is a previous node in the Linked List */
      bool isPrevNull();

      /** Return the next OPMBase object referred to in the Linked List */
      OPMBase* getNext();

      /** Set the next OPMBase object referred to in the Linked List */
      void setNext(OPMBase* newObject);

      /** Return the previous OPMBase object referred to in the Linked List */
      OPMBase* getPrev();

      /** Set the previous OPMBase object referred to in the Linked List */
      void setPrev(OPMBase* newObject);
        
      /** Return the integer poolID that identifies which pool this object belongs to */
      int getPoolID();

      /** Set the integer poolID that identifies which pool this object belongs to */
      void setPoolID(int poolID);

      /** Return a pointer to the object type string for this object */
      const char* getObjectTypeStr();
   
      /** Set the object type string for this object - called upon object creation */
      void setObjectTypeStr(const char* objectType);
      
      /** String'ized debugging tool */
      string toString();

      /**
       * Static method will be called when the pool is created with these objects.
       * This method bootstraps the creation of each object. This method must be
       * static, but SHOULD ALSO BE VIRTUAL so users are forced to implement it.
       * @param initializer - Initializer integer or pointer to initialization
       *      data needed by the object - may be 0 if unused.
       * @returns Pointer to the new OPMBase object.
       */
      static OPMBase* initialize(int initializer);

      /**
       * Pure Virtual method will be called before releasing the object 
       * back into the pool.
       */
      virtual void clean() = 0;

   protected:

      /**
       * Pointer reference to the 'next' OPMBase object in the Linked List
       * implemented in Object Pool.
       */
      OPMBase* next_;

      /**
       * Pointer reference to the 'previous' OPMBase object in the Linked List
       * implemented in Object Pool.
       */
      OPMBase* prev_;

      /** Pointer to the char* object type for this object */
      const char* objectType_;

      /**
       * PoolID used to -self- identify which object pool this object belongs to 
       */
      int poolID_;

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      OPMBase(const OPMBase& rhs);
                                                                                                                       
      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      OPMBase& operator= (const OPMBase& rhs);

};

#endif
