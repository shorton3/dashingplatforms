/******************************************************************************
*
* File name:   SyncObjectPool.cpp
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

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "SyncObjectPool.h"

// Log Manager related includes.
#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: Constructor initializes the pool to a configurable 
//              size and sets the object initializer to be used for all the
//              objects in the pool.
// Design:     
//-----------------------------------------------------------------------------
SyncObjectPool::SyncObjectPool(int objectPoolID, int initialCapacity, 
    double thresholdPercentage, int capacityIncrement, const char* objectType, 
    long objectInitParam, OPM_INIT_PTR bootStrapMethod, OPMGrowthModeType growthMode)
    :ObjectPool::ObjectPool(objectPoolID, initialCapacity, thresholdPercentage, 
      capacityIncrement, objectType, objectInitParam, bootStrapMethod, growthMode)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
SyncObjectPool::~SyncObjectPool()
{
}//end destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reserve an object for use, thus making it unavailable for other
//              tasks.
// Design:      This method automatically monitors the number of in-use objects
//              and will grow the pool when the number exceeds the threshold.
//              If the object pool is specified to be non-resizable, then an 
//              error will be displayed and NULL returned if no available 
//              objects exist.
//-----------------------------------------------------------------------------
OPMBase* SyncObjectPool::reserve(bool blockWaitingForAccess)
{
   if (blockWaitingForAccess == true)
   {
      // Do a blocking acquire
      poolMutex_.acquire();
   }//end if
   else
   {
      // Do a non-blocking acquire
      if (poolMutex_.tryacquire() == ERROR)
      {
         // Acquire failed since some other thread has the lock, so return a NULL
         return NULL;
      }//end if
   }//end else

   OPMBase* object = ObjectPool::reserve();
   poolMutex_.release();
   return object;
}//end reserve


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Release an object back into the pool after using it. This makes
//              the object available again for use in other tasks.
// Design:      This method automatically monitors the number of in-use objects
//              and will shrink the pool when the number drops below the
//              previous increment's threshold value.
//-----------------------------------------------------------------------------
bool SyncObjectPool::release(OPMBase* object, char* callingFileName, int callingLineNumb)
{
   poolMutex_.acquire(); 
   bool result = ObjectPool::release(object, callingFileName, callingLineNumb);
   poolMutex_.release();
   return result;
}//end release


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to test if this pool is empty
// Design:      
//-----------------------------------------------------------------------------
bool SyncObjectPool::isEmpty()
{
   poolMutex_.acquire();
   bool result = ObjectPool::isEmpty();
   poolMutex_.release();
   return result;
}//end isEmpty


//-----------------------------------------------------------------------------
// Method Type:  INSTANCE
// Description:  Method to check if a specified object is in the Free Linked 
//               List or the Used Linked List
// Design:      
//-----------------------------------------------------------------------------
bool SyncObjectPool::containsObject(OPMBase* object)
{
   poolMutex_.acquire();
   bool result = ObjectPool::containsObject(object);
   poolMutex_.release();
   return result;
}//end containsObject


//-----------------------------------------------------------------------------
// Method Type:  INSTANCE
// Description:  Method to set or reset the capacity increment of this Object Pool
// Design:      
//-----------------------------------------------------------------------------
void SyncObjectPool::setCapacityIncrement(int capacityIncrement)
{
   poolMutex_.acquire();
   //set the capacity increment for the Free Linked List
   capacityIncrement_ = capacityIncrement;
   poolMutex_.release();
}//end setCapacityIncrement


//-----------------------------------------------------------------------------
// Method Type:  INSTANCE
// Description:  Method prints the Object Usage Summary for this Object Pool 
//               if DEBUG logs are enabled
// Design:      
//-----------------------------------------------------------------------------
void SyncObjectPool::printUsageSummary()
{
   poolMutex_.acquire();
   ObjectPool::printUsageSummary();
   poolMutex_.release();
}//end printUsageSummary


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

