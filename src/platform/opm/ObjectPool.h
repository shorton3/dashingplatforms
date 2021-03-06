/******************************************************************************
*
* File name:   ObjectPool.h
* Subsystem:   Platform Services
* Description: Implements the Object Pool Class (not thread safe) that is the
*              container for the pooled objects.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_OBJECT_POOL_H_
#define _PLAT_OBJECT_POOL_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <vector>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OPM.h"
#include "OPMBase.h"
#include "OPMLinkedList.h"

#define OPM_DEFAULT_INITIAL_CAPACITY      10

#define OPM_DEFAULT_CAPACITY_INCREMENT    10

#define OPM_DEFAULT_THRESHOLD_PERCENTAGE  .8

#define OPM_MAX_RESIZES                   100

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
 * ObjectPool stores objects of a particular type in the OPM and keeps track
 * of which objects are free and which are in use.
 * <p>
 * Object pool contains methods for reserving and releasing pooled objects
 * and for manipulating the capacity increments of the data structures (pool
 * supports both growing and shrinking based on increment size), computing
 * the count of free objects, setting the threshold and increment values,
 * and performing cleanup.
 * <p>
 * This class is currently NOT THREAD SAFE.
 *
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ObjectPool
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
       * @param objectInitParam - Integer parameter or pointer to initialization
       *        data needed by the object or 0(NULL) if not needed (passed to 
       *        OPMBase::initialize).
       * @param bootStrapMethod pointer to a function or method (OPMBase::initialize)
       *   that will create the object and return a pointer to it
       * @param growthMode Either NO_GROWTH, GROWTH_ALLOWED, or GROW_AND_SHRINK
       */
      ObjectPool(int objectPoolID,
                 int initialCapacity, 
                 double thresholdPercentage,
                 int capacityIncrement, 
                 const char* objectType, 
                 long objectInitParam,
                 OPM_INIT_PTR bootStrapMethod, 
                 OPMGrowthModeType growthMode);

      /** Virtual Destructor */
      virtual ~ObjectPool();


      /**
       * Reserve an object for use, thus making it unavailable for other tasks. This
       * method automatically monitors the number of in-use objects and will grow
       * the pool when the number exceeds the threshold. If the object pool is specified
       * to be non-resizable, then an error will be displayed and NULL returned if
       * no available objects exist.
       * @param blockWaitingForAccess If true, this method will block until an object
       *    becomes available if they are all in use. Otherwise, if false, this method
       *    will return NULL if all objects are in use. Note that this parameter is used
       *    ONLY with SyncObjectPool. It has no meaning here, other than for inheritance.
       */
      virtual OPMBase* reserve(bool blockWaitingForAccess = true);


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
      virtual bool release(OPMBase* object, const char* callingFileName, int callingLineNumb);
      
      /** Return the current capacity of this pool */
      int getCurrentCapacity();

      /** Return the current number of Used Objects in this pool */
      int getCurrentUsedObjects();

      /** Return the object type String being held in this pool */
      const char* getObjectType();

      /** Return the capacity increment size for this pool */
      int getCapacityIncrement();

      /** Return the threshold percentage for this pool */
      double getThresholdPercentage();

      /** Return the initializer for the objects in this pool (if there is one) */
      long getObjectInitParam();
 
      /**
       * Method to test if this pool is empty
       * @return <tt>true</tt> if the Free List is empty<br>
       *         <tt>false</tt> if the Free List is NOT empty
       */
      virtual bool isEmpty();

      /**
       * Method to check if a specified object is in the Free Linked List or the
       * Used Linked List. This method traverses both lists looking for pointer
       * equality.
       * @return <tt>true</tt> if the Object is in the list<br>
       *         <tt>false</tt> if the Object is NOT in the list
       */
      virtual bool containsObject(OPMBase* object);

      /**
       * Method to set or reset the capacity increment of this Object Pool
       * @param capacityIncrement The new capacity increment value
       */
      virtual void setCapacityIncrement(int capacityIncrement);

      /* Method prints the Object Usage Summary for this Object Pool if DEBUG logs are enabled */
      virtual void printUsageSummary();

   protected:

      /** Pool Increment Size which is the number of objects to add when growing */
      int capacityIncrement_;

   private:
 

      /** The number times that the Free Linked List has been enlarged to handle more objects */
      int numberEnlargements_;

      /** Object initializer for all objects in the pool (optional) */
      long objectInitParam_;

      /**
       * Int array for storing the capacity values for each enlargement. By default,
       * space for 30 increases (autoIncreaseFreeList) are preallocated
       */
      vector<int>* historicalCapacityArray_;

      /**
       * Previous threshold value for the last capacity increment -- used to check
       * when removing an object if autoDecreaseFreeList needs to be called
       */
      int previousThresholdCount_;

      /** Initial size of the pool in objects when the pool is created */
      int initialCapacity_;

      /**
       * Percentage of the current capacity at which an automatic capacity increase
       * is triggered 
       */
      double thresholdPercentage_;

      /** Current size of the Object Pool */
      int currentCapacity_;

      /**
       * Total count of objects created by this Object Pool including initial
       * capacity and all objects created during periods of growth.
       */
      long creationCount_;

      /** Current number of objects in the Free List */
      int currentFreeObjects_;

      /** Current number of objects in the Used List */
      int currentUsedObjects_;

      /** Total of all objects used from this pool during runtime */
      long totalUsedObjects_;

      /** Peak number of used objects (at any one time) during runtime */
      int peakUsedObjects_;

      /** Indicates the type of the object stored in this pool */
      char* objectType_;

      /**
       * Linked list of OPMBase objects that are available for use - able to be reserved
       */
      OPMLinkedList* freeList_;

      /**
       * Linked list of OPMBase objects that are in use and will be eventually released
       */
      OPMLinkedList* usedList_;

      /**
       * Method to automatically grow the Free Linked List by instantiating more
       * objects based on the capacityIncrement_ value. This method is called when
       * the reserve Method results in the number of reserved (in-use) objects increasing
       * past the threshold percentage of current total objects (if Growth allowed).
       */
      void autoIncreaseLists();

      /**
       * Method to automatically shrink the Free Linked List by deallocating objects
       * This method is called when the release Method results in the number of 
       * reserved (in-use) objects decreasing below the previous threshold percentage
       * of previous total objects
       */
      void autoDecreaseLists();

      /** Initial Threshold between the initial capacity and its corresponding threshold value */
      int initialThreshold_;

      /** Object PoolID that identifies this object pool in the OPM */
      int objectPoolID_;

      /** 
       * Pointer to a function or method (OPMBase::initialize)
       * that will create the object and return a pointer to it to
       */
      OPM_INIT_PTR bootStrapMethod_;

      /** 
       * Flag to determine if object pool is resizable or not and in what way (grow / shrink /
       * none / both).
       */
      OPMGrowthModeType growthMode_;
};

#endif
