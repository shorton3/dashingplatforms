/******************************************************************************
*
* File name:   ObjectPool.cpp
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

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sstream>

#include <ace/OS_NS_unistd.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ObjectPool.h"

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
ObjectPool::ObjectPool(int objectPoolID, int initialCapacity, 
   double thresholdPercentage, int capacityIncrement, const char* objectType, 
   long objectInitParam, OPM_INIT_PTR bootStrapMethod, OPMGrowthModeType growthMode)
    :capacityIncrement_(OPM_DEFAULT_CAPACITY_INCREMENT),
     numberEnlargements_(0),
     objectInitParam_(objectInitParam),
     previousThresholdCount_(-1),
     initialCapacity_(OPM_DEFAULT_INITIAL_CAPACITY),
     thresholdPercentage_(OPM_DEFAULT_THRESHOLD_PERCENTAGE),
     currentCapacity_(-1),
     creationCount_(-1L),
     currentFreeObjects_(-1),
     currentUsedObjects_(0),
     totalUsedObjects_(0L),
     peakUsedObjects_(0),
     initialThreshold_(0),
     objectPoolID_(objectPoolID),
     bootStrapMethod_(bootStrapMethod),
     growthMode_(growthMode)
{
   //don't trust the user to give us an objectType char* that won't go away - so
   //make a copy
   objectType_ = (char*)malloc(strlen(objectType) + 1);
   objectType_ = strcpy(objectType_, objectType);

   //check the Growth Mode for the pool - if no growth is allowed, then no need to 
   // validate the capacity increment and threshold percentages
   if (growthMode != OPM_NO_GROWTH)
   {
      //set the initial threshold value to be used at all capacity values
      initialThreshold_ = initialCapacity - (int)(initialCapacity * thresholdPercentage);

      //check for valid threshold initializer
      if ( thresholdPercentage > 0)
         thresholdPercentage_ = thresholdPercentage;
      else
         TRACELOG(ERRORLOG, OPMLOG, "Threshold percentage is invalid",0,0,0,0,0,0);
		
      //check for valid capacity Increment initializer
      if ( capacityIncrement > 0)
         capacityIncrement_ = capacityIncrement;
      else 
         TRACELOG(ERRORLOG, OPMLOG, "Capacity Increment not greater than 0",0,0,0,0,0,0);
   }//end if

   //check for valid initial capacity initializer
   if ( initialCapacity > 0)
   {
      initialCapacity_ = initialCapacity;
      currentCapacity_ = initialCapacity;
   }//end if
   else
   {
      TRACELOG(ERRORLOG, OPMLOG, "Initial capacity not greater than 0",0,0,0,0,0,0);
   }//end if

   //initialize the Used and Free Object data structures
   usedList_ = new OPMLinkedList();
   freeList_ = new OPMLinkedList();

   //initialize the Linked List of Free Objects to the initial capacity
   for(int i = 0; i < initialCapacity; i++)
   {
      //create each object by calling its static bootStrap method (initialize)
      // and also pass in the default objectInitializer which may be 0 if
      // none is specified.
      OPMBase* object = (OPMBase*)((*bootStrapMethod_)(objectInitParam));

      // We should NOT allow this to happen
      if (object == NULL)
      {
          TRACELOG(ERRORLOG, OPMLOG, "Object instance creation failed for object number (%d), but continuing with leak",(i+1),0,0,0,0,0);
          continue;
      }//end if
      else if (Logger::getSubsystemLogLevel(OPMLOG) == DEVELOPERLOG)
      {
         TRACELOG(DEBUGLOG, OPMLOG, "OPM Object number (%d) successfully instantiated for poolId (%d)",(i+1),objectPoolID_,0,0,0,0);
      }//end else

      //set the PoolID and objectType string in the object
      object->setPoolID(objectPoolID_);
      object->setObjectTypeStr(objectType_);
 
      //add it to the Free Object List
      freeList_->insertFirst(object);
   }// end for

   //set current Free objects to initial capacity
   currentFreeObjects_ = initialCapacity;

   //increment the creation counter
   creationCount_ = (long)initialCapacity;

   //allocate the historical capacity array of integers and 
   //set the first element in the Historical Capacity Array to the initial capacity
   historicalCapacityArray_ = new vector<int>();
   historicalCapacityArray_->push_back(initialCapacity);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ObjectPool::~ObjectPool()
{
   delete historicalCapacityArray_;

   //clean the Free Object Linked List
   freeList_->deleteListContents();
   delete freeList_;

   //clean the Used Linked List
   usedList_->deleteListContents();
   delete usedList_;
}//end destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reserve an object for use, thus making it unavailable for other
//              threads.
// Design:      This method automatically monitors the number of in-use objects
//              and will grow the pool when the number exceeds the threshold. 
//              If the object pool is specified to be non-resizable, then an 
//              error will be displayed and NULL returned if no available 
//              objects exist.
//-----------------------------------------------------------------------------
OPMBase* ObjectPool::reserve(bool blockWaitingForAccess)
{
   ACE_UNUSED_ARG(blockWaitingForAccess); // ONLY used in SyncObjectPools

   //return NULL if the free list is empty - either Ran-out or wasn't init'ed correctly
   if (freeList_->isEmpty())
   {
      ostringstream ostr;
      ostr << "Pool has no free " << objectType_ << " object types" << ends;
      STRACELOG(ERRORLOG, OPMLOG, ostr.str().c_str());
      return NULL;
   }//end if

   //if the threshold has been reached in any of the Linked Lists, we must
   // increment the capacity
   if ((growthMode_ != OPM_NO_GROWTH) && 
       (currentUsedObjects_ >= (currentCapacity_ - initialThreshold_)))
   {
	   //auto-increase (grow) the Free List
	   autoIncreaseLists();
   }//end if
   //check if not resizable and if the pool is about to run out of objects
   else if (currentUsedObjects_ == (currentCapacity_ - 1))
   {
      ostringstream ostr;
      ostr << "Pool Max Usage (" << currentCapacity_ << ") reached for " << objectType_
           << " object types" << ends;
      STRACELOG(WARNINGLOG, OPMLOG, ostr.str().c_str());
   }//end else if

   //delete an OPMBase object from the head of the Free Linked List
   OPMBase* object = freeList_->removeFirst();

   //decrement the free object count
   currentFreeObjects_ -= 1;

   //increment the object counts
   currentUsedObjects_ += 1;
   //set the peak usage if its larger
   if (currentUsedObjects_ > peakUsedObjects_)
   {
      peakUsedObjects_ = currentUsedObjects_;
      //print a DEBUG log for setting the peak used objects to record the time
      if (Logger::getSubsystemLogLevel(OPMLOG) == DEVELOPERLOG)
      {
         ostringstream ostr;
         ostr << "peakUsedObjects is set for object pool " << objectType_ << " (" << currentUsedObjects_ << ")" << ends;
         STRACELOG(DEBUGLOG, OPMLOG, ostr.str().c_str());
      }//end if
   }//end if

   totalUsedObjects_ += 1L;
   //check to see if the totalUsedObjects_ has overflowed to a negative value. If
   // it has, we reset it to zero so the user does not get confused with a neg.
   if (totalUsedObjects_ < 0L)
      totalUsedObjects_ = 0L;

   //add the OPMBase object to the head of the Used Linked List
   usedList_->insertFirst(object);

   //if in DEBUG mode, print a successful DEBUG log message
   if (Logger::getSubsystemLogLevel(OPMLOG) == DEVELOPERLOG)
   {
      ostringstream ostr;
      ostr << "OPM Object successfully reserved from Pool " << objectType_
           << " in Process (" << getpid() << ")" << ends;
      STRACELOG(DEBUGLOG, OPMLOG, ostr.str().c_str());
   }//end if
   return object;
}//end reserve


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Release an object back into the pool after using it. This makes
//              the object available again for use in other threads.
// Design:      This method automatically monitors the number of in-use objects
//              and will shrink the pool when the number drops below the
//              previous increment's threshold value.
//-----------------------------------------------------------------------------
bool ObjectPool::release(OPMBase* object, const char* callingFileName, int callingLineNumb)
{
   //if in DEBUG mode, perform a check to see if the object belongs in this pool
   if (Logger::getSubsystemLogLevel(OPMLOG) == DEVELOPERLOG)
   {
      if (!containsObject(object))
      {
         ostringstream ostr;
         ostr << "Attempt to release object into WRONG POOL (" << objectPoolID_ 
              << ") - containsObject returns false - aborting! Called from "
              << callingFileName << " (" << callingLineNumb << ") and Process "
              << "(" << getpid() << ")" << ends;
         STRACELOG(ERRORLOG, OPMLOG, ostr.str().c_str());
         return false;
      }//end if
      else if (object->getPoolID() != objectPoolID_)
      {
         ostringstream ostr;
         ostr << "Attempt to release object with poolID (" << object->getPoolID() 
              << ") into WRONG POOL (" << objectPoolID_ << ") - aborting! Called from "
              << callingFileName << " (" << callingLineNumb << ") and Process "
              << "(" << getpid() << ")" << ends;
         STRACELOG(ERRORLOG, OPMLOG, ostr.str().c_str());
         return false;
      }//end if
   }//end if

   //retrieve the OPMBase object in use from the Used List
   object = usedList_->remove(object);

   //check to make sure that the object was found, if not return false and
   // display an ERROR
   if (object == NULL)
   {
      ostringstream oss;
      oss << "Attempt to release object into the WRONG POOL, is already " 
          << "released, or may not have been created via OPM. "
          << "Called from " << callingFileName << " (" 
          << callingLineNumb << ") and Process "  
          << "(" << getpid() << ")" << ends;
      STRACELOG(ERRORLOG, OPMLOG, (char*)oss.str().c_str());
      return false;
   }//end if
   else
   {
      //decrement the used object count
      currentUsedObjects_ -= 1;

      //Clean the Object before returning to the pool
      object->clean();

      //add the OPMBase object back into the Free Linked List
      freeList_->insertFirst(object);

      //increment the free object counter
      currentFreeObjects_ += 1;

      //decrease the number of objects in the Free List and Used List (if Resizable)
      if ((growthMode_ == OPM_GROW_AND_SHRINK) && 
          (previousThresholdCount_ != -1) &&
          (currentUsedObjects_ <= previousThresholdCount_))
      {
         autoDecreaseLists();
      }//end if
   }//end else

   //if in DEBUG mode, print a successful DEBUG log message
   if (Logger::getSubsystemLogLevel(OPMLOG) == DEVELOPERLOG)
   {
      ostringstream oss;
      oss << "OPM Object successfully released into Pool " << objectType_ 
          << ". Called from " << callingFileName << " (" 
          << callingLineNumb << ") and Process "  
          << "(" << getpid() << ")" << ends;
      STRACELOG(DEBUGLOG, OPMLOG, (char*)oss.str().c_str());
   }//end if
   return true;
}//end release


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to automatically grow the Free Linked List by 
//              instantiating more objects based on the capacityIncrement_ 
//              value.
// Design:      This method is called when the reserve Method results in the 
//              number of reserved (in-use) objects increasing past the 
//              threshold percentage of current total objects.
//-----------------------------------------------------------------------------
void ObjectPool::autoIncreaseLists()
{
   //increment enlargement count
   numberEnlargements_ += 1;
   //store the old threshold value in 'previousThresholdCount_'
   previousThresholdCount_ = currentCapacity_ - initialThreshold_;

   //create additional objects and store in the Free List
   for (int i = 0; i < capacityIncrement_; i++)
   {
      //create each object by calling its static bootStrap method (initialize)
      // and also pass in the default objectInitializer which may be 0 if
      // none is specified.
      OPMBase* object = (OPMBase*)((*bootStrapMethod_)(objectInitParam_));

      //set the PoolID and objectType string in the object
      object->setPoolID(objectPoolID_);
      object->setObjectTypeStr(objectType_);

      //add it to the Object List
      freeList_->insertFirst(object);
   }//end for

   //set counters
   currentFreeObjects_ += capacityIncrement_;
   currentCapacity_ += capacityIncrement_;
   creationCount_ += (long)capacityIncrement_;
   //check to see if the creationCount_ has overflowed to a negative value. If
   // it has, we reset it to zero so the user does not get confused with a neg.
   if (creationCount_ < 0L)
      creationCount_ = 0L;

   //store the new capacity in the historical capacity array and increment count
   if (numberEnlargements_ < (int)(historicalCapacityArray_->size()))
      (*historicalCapacityArray_)[numberEnlargements_] = currentCapacity_;
   else //assumes that we're pushing onto the 'numberEnlargements' element
      historicalCapacityArray_->push_back(currentCapacity_);

   //send a DEBUG log msg
   TRACELOG(DEBUGLOG, OPMLOG, "Increase to %d objs, # increases over init capacity = <%d>", 
      currentCapacity_, numberEnlargements_,0,0,0,0);
}//end autoIncreaseLists


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to automatically shrink the Free Linked List by 
//              delinking and deallocating objects. 
// Design:      This method is called when the release Method results in the 
//              number of reserved (in-use) objects decreasing below the 
//              previous threshold percentage of previous total objects
//-----------------------------------------------------------------------------
void ObjectPool::autoDecreaseLists()
{
   //only reduce if enlargements in capacity have already been made
   if (numberEnlargements_ <= 0)
      return;

   //retrieve the new capacity from the historical capacity array and decrement count
   int newCapacity = (*historicalCapacityArray_)[numberEnlargements_ -= 1];

   //send a DEBUG log msg
   TRACELOG(DEBUGLOG, OPMLOG, "Reduce to %d objs, # increases over init capacity=%d", 
      newCapacity, numberEnlargements_,0,0,0,0);

   //calculate the new previous threshold to watch for (unless we're at bottom)
   if (numberEnlargements_ > 0)
   {
      previousThresholdCount_ = 
         (*historicalCapacityArray_)[numberEnlargements_ - 1] - initialThreshold_;
   }//end if
   else //numberEnlargements_ == 0
      previousThresholdCount_ = -1;

   //based on the difference between the new (lower) capacity and the current
   // capacity, we know how many objects to delete from the Free List
   int numbToDelete = currentCapacity_ - newCapacity;

   for (int i = 0; i < numbToDelete; i++)
   {
      //delete from the top of the list--nulled out and deallocated
      OPMBase* object = freeList_->removeFirst();
      delete object;
   }//end for

   //update the counters
   currentCapacity_ = newCapacity;
   currentFreeObjects_ -= numbToDelete;
}//end autoDecreaseFreeList


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the current capacity of this pool
// Design:      
//-----------------------------------------------------------------------------
int ObjectPool::getCurrentCapacity()
{
   return currentCapacity_;
}//end getCurrentCapacity


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the current number of Used Objects in this pool
// Design:      
//-----------------------------------------------------------------------------
int ObjectPool::getCurrentUsedObjects()
{
   return currentUsedObjects_;
}//end getCurrentUsedObjects


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the object type String being held in this pool
// Design:      
//-----------------------------------------------------------------------------
const char* ObjectPool::getObjectType()
{
   return objectType_;
}//end getObjectType


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the capacity increment size for this pool
// Design:      
//-----------------------------------------------------------------------------
int ObjectPool::getCapacityIncrement()
{
   return capacityIncrement_;
}//end getCapacityIncrement


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the threshold percentage for this pool
// Design:      
//-----------------------------------------------------------------------------
double ObjectPool::getThresholdPercentage()
{
   return thresholdPercentage_;
}//end getThresholdPercentage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the initializer for the objects in this pool (if there 
//              is one)
// Design:      
//-----------------------------------------------------------------------------
long ObjectPool::getObjectInitParam()
{
   return objectInitParam_;
}//end getObjectInitParam


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to test if this pool is empty
// Design:      
//-----------------------------------------------------------------------------
bool ObjectPool::isEmpty()
{
   return freeList_->isEmpty();
}//end isEmpty


//-----------------------------------------------------------------------------
// Method Type:  INSTANCE
// Description:  Method to check if a specified object is in the Free Linked 
//               List or the Used Linked List
// Design:      
//-----------------------------------------------------------------------------
bool ObjectPool::containsObject(OPMBase* object)
{
   //linearly search the Used and Free Linked List
   return (freeList_->containsObject(object) || usedList_->containsObject(object));
}//end containsObject


//-----------------------------------------------------------------------------
// Method Type:  INSTANCE
// Description:  Method to set or reset the capacity increment of this Object Pool
// Design:      
//-----------------------------------------------------------------------------
void ObjectPool::setCapacityIncrement(int capacityIncrement)
{
   //set the capacity increment for the Free Linked List
   capacityIncrement_ = capacityIncrement;
}//end setCapacityIncrement


//-----------------------------------------------------------------------------
// Method Type:  INSTANCE
// Description:  Method prints the Object Usage Summary for this Object Pool 
// Design:      
//-----------------------------------------------------------------------------
void ObjectPool::printUsageSummary()
{
   ostringstream oss;
   oss << "************ Object Pool Summary **********************\n\n" 
       << "ObjectType: " << objectType_ << "\n"
       << "Object Pool ID: " << objectPoolID_ << "\n"
       << "Initial Capacity Count: " << initialCapacity_ << "\n";

   if (objectInitParam_ == 0)
      oss << "Object Initializer: NONE\n";
   else
      oss << "Object Initializer located at " << (long)objectInitParam_ << "\n";

   if (growthMode_ == OPM_NO_GROWTH)
      oss << "Growth Mode: NO GROWTH\n";
   else if (growthMode_ == OPM_GROWTH_ALLOWED)
      oss << "Growth Mode: ONLY GROW\n";
   else if (growthMode_ == OPM_GROW_AND_SHRINK)
      oss << "Growth Mode: GROW AND SHRINK\n";

   oss << "Current Capacity Increment: " << capacityIncrement_ << "\n"
       << "Threshold Percentage: " << thresholdPercentage_ << "\n"
       << "Number of expansions: " << numberEnlargements_ << "\n"
       << "Current Capacity: " << currentCapacity_ << "\n"
       << "Total Objects Created: " << creationCount_ << "\n"
       << "Current Used Count: " << currentUsedObjects_ << "\n"
       << "Current Free Count: " << currentFreeObjects_ << "\n"
       << "Total Used Count: " << totalUsedObjects_ << "\n"
       << "Peak Used Object Count: " << peakUsedObjects_ << "\n"
       << "*******************************************************\n"
       << ends;
   STRACELOG(DEBUGLOG, OPMLOG, oss.str().c_str());
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
