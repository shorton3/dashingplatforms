/******************************************************************************
*
* File name:   OPMLinkedList.h
* Subsystem:   Platform Services
* Description: Implements the storage structure that the pooled objects are
*              stored within.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_OPM_LINKED_LIST_H_
#define _PLAT_OPM_LINKED_LIST_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OPMBase.h"

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
 * This utility class implements a doubly linked list with both a head and
 * tail pointers for storing OPMBase objects. All list operations are done 
 * by manipulating pointers since OPMBase stores the
 * references for the 'NEXT' and 'PREV' pointers.
 * <p>
 * This class is currently NOT THREAD SAFE.
 *
 * $Author: Stephen Horton$
 * $Revision: 1$
 */
class OPMLinkedList
{
   public:

      /** Constructor */
      OPMLinkedList();

      /** Virtual Destructor */
      virtual ~OPMLinkedList();

      /**
       * Remove an OPMBase object from the Head (First) of the Linked List and
       * return the object back to the user.
       * @return OPMBase pointer to object or null if the List is empty
       */
      OPMBase* removeFirst();

      /**
       * Remove a specified OPMBase object from the Linked List by manipulating
       * the previous and next pointers and return the object back to the user.
       * @return OPMBase pointer to object or null if the List is empty
       */
      OPMBase* remove(OPMBase* object);

      /**
       * Insert an OPMBase object at the Head (First) of the Linked List
       * @param object Pointer to OPMBase object to insert
       */
      void insertFirst(OPMBase* object);

      /**
       * Insert an OPMBase object at the Tail (Last) of the Linked List
       * @param object Pointer to OPMBase object to insert
       */
      void insertLast(OPMBase* object);

      /** Return the size of the List */
      int getListSize();

      /** Check if the list is currently empty */
      bool isEmpty();

      /** Walk the linked list and check to see if the object is a member */
      bool containsObject(OPMBase* object);

      /** 
       * Delete the contents of the list - does not delete the list
       * objects, but rather deletes the list references.
       */
      void deleteListContents();

      /** Display the contents of the list */
      string toString();

   protected:

   private:
       
      /** Reference to the beginning of the Linked List of OPMBase objects */
      OPMBase* head_;

      /** Reference to the end of the Linked List of OPMBase objects */
      OPMBase* tail_;

      /** Current number of objects in the list */
      int listSize_;
};

#endif
