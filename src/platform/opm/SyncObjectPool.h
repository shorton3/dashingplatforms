/******************************************************************************
*
* File name:   SyncObjectPool.h
* Subsystem:   Platform Services
* Description: Implements the Object Pool Class (thread safe!) that is the
*              container for the pooled objects.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_SYNC_OBJECT_POOL_H_
#define _PLAT_SYNC_OBJECT_POOL_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Synch.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ObjectPool.h"

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

/**
 * SyncObjectPool provides a wrapper class for making Object Pool thread
 * safe. It is the responsibility of the developer to determine if the
 * Object Pool will be accessed from multiple threads.
 * <p>
 * Object pool contains methods for reserving and releasing pooled objects
 * and for manipulating the capacity increments of the data structures (pool
 * supports both growing and shrinking based on increment size), computing
 * the count of free objects, setting the threshold and increment values,
 * and performing cleanup.
 * <p>
 *
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class SyncObjectPool : public ObjectPool
{
   public:

      /**
       * Constructor initializes the pool to a configurable size and sets
       * the individual object's initializer to be stored in the Pool.
       * @param objectPoolID Object PoolID that identifies this object pool in the OPM
       * @param initialCapacity Starting size of the pool in objects
       * @param thresholdPercentage Percentage of current capacity to trigger
       *        automatic growth.
       * @param capacityIncrement Number of objects to add to the current size when
       *        increasing capacity
       * @param objectType const char* describing the class object stored in this pool
       * @param objectInitParam - Long parameter or pointer to initialization
       *        data needed by the object or 0(NULL) if not needed (passed to 
       *        OPMBase::initialize).
       * @param bootStrapMethod pointer to a function or method (OPMBase::initialize)
       *        that will create the object and return a pointer to it
       * @param growthMode Either NO_GROWTH, GROWTH_ALLOWED, or GROW_AND_SHRINK
       */
      SyncObjectPool(int objectPoolID,
                 int initialCapacity, 
                 double thresholdPercentage,
                 int capacityIncrement, 
                 const char* objectType, 
                 long objectInitParam,
                 OPM_INIT_PTR bootStrapMethod, 
                 OPMGrowthModeType growthMode);

      /** Virtual Destructor */
      virtual ~SyncObjectPool();

      /**
       * Reserve an object for use, thus making it unavailable for other tasks. This
       * method automatically monitors the number of in-use objects and will grow
       * the pool when the number exceeds the threshold. If the object pool is specified
       * to be non-resizable, then an error will be displayed and NULL returned if
       * no available objects exist.
       * @param blockWaitingForAccess If true, this method will block until an object
       *    becomes available if they are all in use. Otherwise, if false, this method
       *    will return NULL if all objects are in use.
       */
      OPMBase* reserve(bool blockWaitingForAccess = true);


      /**
       * Release an object back into the pool after using it. This makes the
       * object available again for use in other tasks. This method automatically
       * monitors the number of in-use objects and will shrink the pool when the
       * number drops below the previous increment's threshold value.
       * @param object Object to release back into the pool.
       * @param callingFileName - Specifies the source file from which the object 
       *    is being released (this will be printed as a DEBUG log using a MACRO)
       * @param callingLineNumb - Specifies the source line from which the object 
       *    is being released (this will be printed as a DEBUG log using a MACRO)
       * @return <tt>true</tt> if the specified Object was successfully released<br>
       *         <tt>false</tt> if the Object Reference was not found in the USED
       *         data structure (could be wrong pool, is already released, or was
       *         not created via OPM)
       */
      bool release(OPMBase* object, char* callingFileName, int callingLineNumb);
      
      /**
       * Method to test if this pool is empty
       * @return <tt>true</tt> if the Free List is empty<br>
       *         <tt>false</tt> if the Free List is NOT empty
       */
      bool isEmpty();

      /**
       * Method to check if a specified object is in the Free Linked List or the
       * Used Linked List. This method traverses both lists looking for pointer
       * equality.
       * @return <tt>true</tt> if the Object is in the list<br>
       *         <tt>false</tt> if the Object is NOT in the list
       */
      bool containsObject(OPMBase* object);

      /**
       * Method to set or reset the capacity increment of this Object Pool
       * @param capacityIncrement The new capacity increment value
       */
      void setCapacityIncrement(int capacityIncrement);

      /* Method prints the Object Usage Summary for this Object Pool if DEBUG logs are enabled */
      void printUsageSummary();


   protected:


   private:

      /** Mutex that controls task access to this object pool */
      ACE_Recursive_Thread_Mutex poolMutex_;
};

#endif
