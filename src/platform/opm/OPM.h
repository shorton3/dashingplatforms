/******************************************************************************
*
* File name:   OPM.h
* Subsystem:   Platform Services
* Description: Implements the main API interface for OPM. This interface 
*              allows developers to create pools of objects and check-in and
*              check-out from these pools.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifndef _PLAT_OPM_MGR_H_
#define _PLAT_OPM_MGR_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <vector>
#include <ace/Synch.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OPMBase.h"

// Use a MACRO wrapper around functions to determine/track the calling Class
// and Method names
#define OPM_RELEASE(object) \
OPM::releaseObject(object, __FILE__, __LINE__); 

#define OPM_RESERVE(objectPoolID) \
OPM::reserveObject(objectPoolID); 

#define OPM_RESERVE_NONBLOCKED(objectPoolID) \
OPM::reserveObject(objectPoolID, false);

// Modes allowed for Growing and Shrinking Object Pool capacities
typedef enum { OPM_NO_GROWTH = -1, 
               OPM_GROWTH_ALLOWED = -2,
               OPM_GROW_AND_SHRINK = -3
             } OPMGrowthModeType;

// Function pointer that returns OPMBase*
typedef OPMBase* (*OPM_INIT_PTR)(int);

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

class ObjectPool;

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
 * OPM (Object Pool Manager) manages the creation of a pool of reusable objects
 * that can be reserved for use within the program and made available again for
 * reuse. 
 * <p>
 * OPM contains overloaded methods for creating the object pool, getting the
 * objects from the pool and releasing the objects back into the pool. NOTE: There
 * currently is no support for deleting/destroying existing pools and their
 * objects.
 * <p>
 *
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class OPM
{

   /**
    * Friend function prints the usage summary for all Object Pools being 
    * managed within the OPM.
    */
   friend void cliPrintAllPoolsSummary();

   /**
    * Friend function prints the usage summary for a particular Object Pool.
    * @param objectPoolID integer used by applications in referencing this 
    * particular object pool for retrieving objects (this is returned by 
    * createPool)
    */
   friend void cliPrintOPMPoolSummary(int objectPoolID);

   /**
    * Friend function tests all of the pools and prints the statistics per
    * Object Pool for objects which HAVE NOT been properly returned/released 
    * into the OPM.
    */
   friend void checkForUnreleasedOPMObjects();


   public:

      /**
       * initializes the Object Pool Manager (OPM).
       */
      static bool initialize();

      /**
       * Method to be called when the node is shutdown. This method will reset
       * all Objects to <tt> NULL </tt>
       */
      static void shutdown();

      /**
       * Method for Creating an Object Pool of Specified Object Type, Object Count, 
       * Initial capacity, capacity increment, threshold, and growth Mode.
       * <p>
       * Note that this method allows for an object Initializer to be specified for
       * all objects in the pool.
       * <p>
       * Note that if the Object Pool already exists (ie. was created earlier by 
       * another part of the application), then the existing object Pool ID is returned.
       * <p>
       * @param objectType Type of the object to be Created
       * @param objectInitParam long or pointer to initialization data needed
       *   by each of the objects in the pool, or NULL if none is needed
       * @param bootStrapMethod pointer to a function or method (OPMBase::initialize)
       *   that will create the object and return a pointer to it
       * @param thresholdPercentage double representing the percentage of the initial
       *   capacity above which to autoincrease the pool size.
       * @param capacityIncrement integer number of objects to add to the pool when
       *   autoincrementing the pool capacity
       * @param initialSize initial capacity of objects to create within the pool
       * @param threadSafe Boolean to determine if the ObjectPool should be created
       *   as thread safe or not
       * @param growthMode Either NO_GROWTH, GROWTH_ALLOWED, or GROW_AND_SHRINK
       * @return integer objectPoolID used by applications in referencing this particular
       *   object pool for retrieving objects (or ERROR if pool already exists for
       *   the specified objectType and initializer).
       */
      static int createPool(const char* objectType, long objectInitParam,
         OPM_INIT_PTR bootStrapMethod, double thresholdPercentage, int capacityIncrement, 
         int initialSize, bool threadSafe, OPMGrowthModeType growthMode);

      /**
       * Method to verify if an object pool has been created for the specified 
       * Object type
       * @param objectType type string of the objects being pooled
       * @param objectInitParam long or pointer to initialization data needed
       *   by each of the objects in the pool
       * @return <tt>true</tt> if the named object pool exists <br>
       *		     <tt>false</tt> if the named object pool does not exist
       */
      static bool isPoolCreated(const char* objectType, long objectInitParam);

      /**
       * Method to return the Object Pool ID for an existing object Pool.
       * @param objectType type string of the objects being pooled
       * @param objectInitParam long or pointer to initialization data needed
       *   by each of the objects in the pool
       * @return the Object Pool ID for an existing object Pool; otherwise,
       *   ERROR if the object pool does not exist.
       */
      static int getObjectPoolID(const char* objectType, long objectInitParam);
 
      /**
       * Method to retrieve an object from a specified pool being managed within OPM.
       * @param objectPoolID integer used by applications in referencing this particular
       *   object pool for retrieving objects (this is returned by createPool)
       * @param blockWaitingForAccess If true, this method will block until an object
       *    becomes available if they are all in use. Otherwise, if false, this method
       *    will return NULL if all objects are in use.
       * @return OPMBase object if the specified object type exists within OPM;
       *   otherwise, it returns <tt>NULL</tt>. NOTE that it can also return NULL
       *   if the user has requested a non-blocking call (blockWaitingForAccess = false)<br>
       */
      static OPMBase* reserveObject(int objectPoolID, bool blockWaitingForAccess = true);

      /**
       * Method for releasing the specified object back into the OPM pool after it
       * is no longer in-use.
       * @param object Pointer to the Object to be released back into the Pool
       * @param callingFileName - Specifies the source file from which the object 
       *    is being released (this will be printed as a DEBUG log using a MACRO)
       * @param callingLineNumb - Specifies the source line from which the object 
       *    is being released (this will be printed as a DEBUG log using a MACRO)
       * @return <tt>true</tt> if the specified Object was successfully released<br>
       *         <tt>false</tt> if the Object Reference was not found in the Pool's
       *         USED data structure (could be wrong pool, is already released, or)
       *         was not created via OPM)
       */
      static bool releaseObject(OPMBase* object, 
                                const char* callingFileName, 
                                int callingLineNumb);

      /**
       * Method to test if an Object was created by the OPM.
       * Remember that for this to be called from Multiple threads, the
       * object pool must be thread safe.
       * @param object OPMBase Object pointer to test
       * @return <tt>true</tt> if the specified Object was created by OPM <br>
       *         <tt>false</tt> if the specified Object was created elsewhere
       */
      static bool isCreatedByOPM(OPMBase* objectRef);

      /**
       * Method sets the capacityIncrement for the specified Object Pool. This
       * value is either initially specified as a parameter to the createPool()
       * method or is chosen by the default constructor of ObjectPool. <p>
       * @param objectPoolID integer used by applications in referencing this particular
       *   object pool for retrieving objects (this is returned by createPool)
       * @param capacityIncrement Number of objects to add to the pool when increasing
       * @return <tt>true</tt> if the capacity increment was successfully changed<br>
       *         <tt>false</tt> if setting the capacity increment failed
       */
      static bool setPoolCapacityIncrement(int objectPoolID, int capacityIncrement);

      /**
       * Get the Pool Capacity Increment value which is the number of objects to
       * add to the pool when the threshold value is reached. <p>
       * @param objectPoolID integer used by applications in referencing this particular
       *   object pool for retrieving objects (this is returned by createPool)
       * @return Returns the Pool Capacity Increment or -1 if there is an error
       */
      static int getPoolCapacityIncrement(int objectPoolID);

      /**
       * Get the Pool Threshold Percentage value which is the factor (double) of the
       * current capacity at which to increase the capacity. <p>
       * @param objectPoolID integer used by applications in referencing this particular
       *   object pool for retrieving objects (this is returned by createPool)
       * @return Returns the Pool threshold percentage as a float or 0 if there is
       * an error
       */
      static double getPoolThresholdPercentage(int objectPoolID);

      /**
       * Get the Pool's object initializer (or 0/NULL if there is none).
       * @param objectPoolID integer used by applications in referencing this particular
       *   object pool for retrieving objects (this is returned by createPool)
       * @return Returns the integer initializer, pointer to data, or 0(NULL)
       *   if there is none
       */
      static int getPoolObjectInitParam(int objectPoolID);

      /**
       * Method prints the usage summary for all Object Pools being managed within
       * the OPM. This method is called by default when the shutdown method is
       * called.
       */
      static void printAllPoolsSummary();

      /**
       * Method prints the usage summary for a particular Object Pool.
       * @param objectPoolID integer used by applications in referencing this particular
       *   object pool for retrieving objects (this is returned by createPool)
       */
      static void printPoolSummary(int objectPoolID);

      /**
       * Method tests all of the pools and prints the statistics per Object Pool for
       * objects which HAVE NOT been properly returned/released into the OPM.
       */
      static void checkForUnreleasedObjects();


   protected:

   private:

      /** Vector datastructure for storing Object Pools into -- this is OPM */
      static vector<ObjectPool*>* poolVector_;

      /** Non-recursive Mutex that controls access / insertion into the OPM - Note: deletion of pools
          is not supported */
      static ACE_Thread_Mutex poolVectorMutex_;

      /** Flag for determining if OPM is already initialized */
      static bool isInitialized_;
};

#endif
