/******************************************************************************
*
* File name:   OPM.cpp
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

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sstream>
#include <ace/Task.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OPM.h"
#include "OPMBase.h"
#include "ObjectPool.h"
#include "SyncObjectPool.h"

// Log Manager related includes.
#include "platform/logger/Logger.h"

// Common Defines
#include "platform/common/Defines.h"

using namespace std;

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/** Vector datastructure for storing Object Pools into -- this is OPM */
vector<ObjectPool*>* OPM::poolVector_ = NULL;

/** Non-recursive Mutex that controls insertion into the OPM - Note: deletion of pools
    is not supported */
ACE_Thread_Mutex OPM::poolVectorMutex_;

/** Flag for determining if OPM is already initialized */
bool OPM::isInitialized_ = false;

/* From the C++ FAQ, create a module-level identification string using a compile 
   define - BUILD_LABEL must have NO spaces passed in from the make command 
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)Object Pool Manager"
   "\n   Build Label: " XstrConvert(BUILD_LABEL) 
   "\n   Compile Time: " __DATE__ " " __TIME__;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Initialize the OPM
// Design:     
//-----------------------------------------------------------------------------
bool OPM::initialize()
{
   if (isInitialized_ == false)
   {
      //Create the Object Pool Manager (OPM)
      poolVector_ = new vector<ObjectPool*>();

      // Set the flag to Initialized=true
      isInitialized_ = true;

      //Log the initialization
      TRACELOG(DEBUGLOG, OPMLOG, "OPM Initialized successfully", 0,0,0,0,0,0);
   }//end if
   else
   {
      TRACELOG(DEBUGLOG, OPMLOG, "OPM already Initialized", 0,0,0,0,0,0);
   }//end else

   return true;
}//end initialize



//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to be called when the node is shutdown. This method will
//              reset all Objects to NULL
// Design:     
//-----------------------------------------------------------------------------
void OPM::shutdown()
{
   //Print the Summary of statistics for all Object Pools
   printAllPoolsSummary();
    
   //traverse the OPM vector and call delete on each pool
   poolVectorMutex_.acquire();
   for (unsigned int i = 0; i < poolVector_->size(); i++)
   {
      delete (*poolVector_)[i];
   }//end for
    
   //delete the OPM vector
   delete poolVector_;
   poolVectorMutex_.release();

   //delete the vector protection mutex
   //delete &poolVectorMutex_;
}//end shutdown


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method for Creating an Object Pool of Array Objects of 
//              specified size, object Type, Initial Capacity, Capacity 
//              Increment, and Threshold Percentage.
// Design:     
//-----------------------------------------------------------------------------
int OPM::createPool(const char* objectType, long objectInitParam, 
   OPM_INIT_PTR bootStrapMethod, double thresholdPercentage, int capacityIncrement, 
   int initialSize, bool threadSafe, OPMGrowthModeType growthMode)
{
   int objectPoolID = ERROR;
   ObjectPool* newPool = NULL;

   //verify that the OPM has been initialized
   if (isInitialized_ == false)
   {
      TRACELOG(ERRORLOG, OPMLOG, "OPM has yet to be initialized",0,0,0,0,0,0);
      return ERROR;
   }//end if

   //verify that the bootStrap method used to create the objects is not NULL
   if (!bootStrapMethod)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Bootstrap method passed to createPool is NULL",0,0,0,0,0,0);
      return ERROR;
   }//end if   

   //make sure that requested Object Pool name does not exist in the OPM already.
   // If it does, then we just want to return the ID of the existing pool.
   if ((objectPoolID = OPM::getObjectPoolID(objectType, objectInitParam)) != ERROR)
   {
      ostringstream ostr;
      ostr <<  "Pool already exists for " << objectType << ". Returning existing Pool ID ("
           << objectPoolID << ")" << ends;
      STRACELOG(WARNINGLOG, OPMLOG, ostr.str().c_str());

      return objectPoolID;
   }//end if

   //Mutex to protect insertion into the vector of object pools
   poolVectorMutex_.acquire();

   //get the vector index to be the new objectPoolID
   objectPoolID = poolVector_->size();

   //create the new Object Pool based on whether or not it should be Thread safe    
   if (threadSafe)
   {
      newPool = new SyncObjectPool(objectPoolID, initialSize, thresholdPercentage,
        capacityIncrement, objectType, objectInitParam, bootStrapMethod, growthMode);
   }//end if
   else
   {
      newPool = new ObjectPool(objectPoolID, initialSize, thresholdPercentage,
        capacityIncrement, objectType, objectInitParam, bootStrapMethod, growthMode);
   }//end else

   //debug log message 
   if (newPool != NULL)
   {
      //store the new Object Pool in the OPM vector using the objectType 
      // and return the new PoolID
      poolVector_->push_back(newPool); 

      //Release vector protection mutex 
      poolVectorMutex_.release();

      //successfully created and populated the pool
      ostringstream ostr;
      ostr << "Object Pool successfully created for " << objectType << ends;
      STRACELOG(DEBUGLOG, OPMLOG, ostr.str().c_str());
      return objectPoolID;
   }//end if
   else
   {
      //Release vector protection mutex
      poolVectorMutex_.release();

      //successfully created and populated the pool
      ostringstream ostr;
      ostr << "Error Creating Object Pool for " << objectType << " objects" << ends;
      STRACELOG(ERRORLOG, OPMLOG, ostr.str().c_str());
      return ERROR;
   }//end else
}//end createPool
  

//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to verify if an object pool has been created for the 
//              specified Object type.
// Design:     
//-----------------------------------------------------------------------------
bool OPM::isPoolCreated(const char* objectType, long objectInitParam)
{
   bool result = false;

   poolVectorMutex_.acquire();

   for (unsigned int i = 0; i < poolVector_->size(); i++)
   {
      //check for both the objectType and the initializer
      if ((strcmp(objectType, (*poolVector_)[i]->getObjectType()) == 0) &&
          (objectInitParam == (*poolVector_)[i]->getObjectInitParam()))
      {
         result = true;
         break;
      }//end if
   }//end for

   poolVectorMutex_.release();
   return result;
}//end isPoolCreated


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to return the Object Pool ID for an existing object Pool.
//              Returns ERROR if the object pool does not exist.
// Design:
//-----------------------------------------------------------------------------
int OPM::getObjectPoolID(const char* objectType, long objectInitParam)
{
   poolVectorMutex_.acquire();

   for (unsigned int i = 0; i < poolVector_->size(); i++)
   {
      //check for both the objectType and the initializer
      if ((strcmp(objectType, (*poolVector_)[i]->getObjectType()) == 0) &&
          (objectInitParam == (*poolVector_)[i]->getObjectInitParam()))
      {
         poolVectorMutex_.release();
         return i;
      }//end if
   }//end for

   poolVectorMutex_.release();
   return ERROR;
}//end getObjectPoolID


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to retrieve an object from a specified pool being
//              managed within OPM.
// Design:     
//-----------------------------------------------------------------------------
OPMBase* OPM::reserveObject(int objectPoolID, bool blockWaitingForAccess)
{
   poolVectorMutex_.acquire();

   //check the poolID
   if (objectPoolID > ((int)(poolVector_->size()) - 1))
   { 
      TRACELOG(ERRORLOG, OPMLOG, "Invalid object poolID",0,0,0,0,0,0);
      return NULL;
   }//end if

   //attempt to get the specified pool from OPM
   ObjectPool* pool = (*poolVector_)[objectPoolID];

   poolVectorMutex_.release();

   //generate an ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Object Pool ID %d does not exist in OPM", objectPoolID,0,0,0,0,0);
      return NULL;
   }//end if

   //retrieve a free object from the specified pool to return to the user - Note
   // that ObjectPool->reserve already returns an OPMBase type object. Check to see
   // if the user is requesting a non-blocking call
   OPMBase* object =  pool->reserve(blockWaitingForAccess);
   return object; 
}//end reserveObject


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method for releasing objects back into the OPM pool
//              after they are no longer in-use.
// Design:     
//-----------------------------------------------------------------------------
bool OPM::releaseObject(OPMBase* object, const char* callingFileName, 
   int callingLineNumb)
{
   //make sure that the object is not NULL
   if (object == NULL)
   {
      ostringstream oss;
      oss << "Attempting to release null object into OPM. "
          << "Called from " << callingFileName << " (" 
          << callingLineNumb << ")" << ends;
      STRACELOG(ERRORLOG, OPMLOG, oss.str().c_str());
      return false;
   }//end if

   // First, see if the pool Id has been initialized in the OPMBase object or not
   int poolId = object->getPoolID();
   if (poolId <= UNKNOWN_POOLID)
   {
      ostringstream oss;
      oss << "Attempting to release non-OPM owned object into OPM. "
          << "Called from " << callingFileName << " ("
          << callingLineNumb << ")" << ends;
      STRACELOG(ERRORLOG, OPMLOG, oss.str().c_str());
      return false;
   }//end if

   poolVectorMutex_.acquire();

   //retrieve the pool from the OPM
   ObjectPool* pool = (*poolVector_)[poolId];

   poolVectorMutex_.release();

   //check to make sure the Pool has been stored in the OPM for this object
   //generate an ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      ostringstream oss;
      oss << "Object Pool ID " << poolId << " does not exist in OPM. "
          << "Called from " << callingFileName << " (" 
          << callingLineNumb << ")" << ends;
      STRACELOG(ERRORLOG, OPMLOG, oss.str().c_str());
      return false;
   }//end if
    
   //release the object back into the pool
   bool result = pool->release(object, callingFileName, callingLineNumb);
   return result;
}//end releaseObject
  

//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method to test if an Object was created by the OPM.
// Design: Remember that for this to be called from Multiple threads, the 
//    object pool must be thread safe     
//-----------------------------------------------------------------------------
bool OPM::isCreatedByOPM(OPMBase* object)
{
   // First, see if the pool Id has been initialized in the OPMBase object or not
   int poolId = object->getPoolID();
   if (poolId <= UNKNOWN_POOLID)
   {
      return false;
   }//end if

   poolVectorMutex_.acquire();
 
   //get the correct Pool from the OPM
   ObjectPool* pool = (*poolVector_)[poolId];

   poolVectorMutex_.release();

   //check to make sure the Pool has been stored in the OPM for this object
   //generate an ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Object Pool ID %d does not exist in OPM", object->getPoolID(),0,0,0,0,0);
      return false;
   }//end if
    
   //ask the Pool if it contains the reference
   bool result = pool->containsObject(object);
   return result;
}//end isCreatedByOPM


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method sets the capacityIncrement for the specified Object Pool. 
//              This value is either initially specified as a parameter to the 
//              createPool() Method or is chosen by the default constructor of
//              ObjectPool.
// Design:     
//-----------------------------------------------------------------------------
bool OPM::setPoolCapacityIncrement(int objectPoolID, int capacityIncrement)
{
   poolVectorMutex_.acquire();

   //check the poolID
   if (objectPoolID > (((int)poolVector_->size()) - 1))
   {
      TRACELOG(ERRORLOG, OPMLOG, "Invalid object poolID",0,0,0,0,0,0);
      return false;
   }//end if

   //check for a valid capacity increment; otherwise generate a ERROR
   if (capacityIncrement <= 0)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Capacity Increment %d is not valid", capacityIncrement, 0,0,0,0,0);
      return false;
   }//end if

   //get the correct Pool from the OPM
   ObjectPool* pool = (*poolVector_)[objectPoolID];

   poolVectorMutex_.release();

   //check to make sure the Pool has been stored in the OPM for this object
   //generate an ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Object Pool ID %d does not exist in OPM", objectPoolID, 0,0,0,0,0);
      return false;
   }//end if

   //set the capacity Increment in the pool - DOES THIS NEED TO BE THREAD PROTECTED
   pool->setCapacityIncrement(capacityIncrement);
   return true;
}//end setPoolIncrementPercentage
  
  
//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Get the Pool Capacity Increment value which is the number of
//              objects to add to the pool when the threshold value is reached.
// Design:     
//-----------------------------------------------------------------------------
int OPM::getPoolCapacityIncrement(int objectPoolID)
{
   poolVectorMutex_.acquire();

   //check the poolID
   if (objectPoolID > (((int)poolVector_->size()) - 1))
   {
      TRACELOG(ERRORLOG, OPMLOG, "Invalid object poolID",0,0,0,0,0,0);
      return ERROR;
   }//end if

   //get the correct Pool from the OPM
   ObjectPool* pool = (*poolVector_)[objectPoolID];

   poolVectorMutex_.release();

   //check to make sure the Pool has been stored in the OPM for this object
   //generate an ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Object Pool ID %d does not exist in OPM", objectPoolID, 0,0,0,0,0);
      return ERROR;
   }//end if

   //get the capacity Increment from the pool
   return pool->getCapacityIncrement();
}//end getPoolIncrementPercentage


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Get the Pool Threshold Percentage value which is the factor 
//              (double) of the current capacity at which to increase the 
//              capacity.
// Design:     
//-----------------------------------------------------------------------------
double OPM::getPoolThresholdPercentage(int objectPoolID)
{
   poolVectorMutex_.acquire();

   //check the poolID
   if (objectPoolID > (((int)poolVector_->size()) - 1))
   {
      TRACELOG(ERRORLOG, OPMLOG, "Invalid object poolID",0,0,0,0,0,0);
      return 0.0;
   }//end if

   //get the correct Pool from the OPM
   ObjectPool* pool = (*poolVector_)[objectPoolID];

   poolVectorMutex_.release();

   //check to make sure the Pool has been stored in the OPM for this object
   //generate an ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Object Pool ID %d does not exist in OPM", objectPoolID,0,0,0,0,0);
      return 0.0;
   }//end if
    
   //get the threshold from the pool
   return pool->getThresholdPercentage();
}//end getPoolThresholdPercentage


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Get the Pool's object initializer (or 0/NULL if there is none).
// Design:     
//-----------------------------------------------------------------------------
int OPM::getPoolObjectInitParam(int objectPoolID)
{
   poolVectorMutex_.acquire();

   //check the poolID
   if (objectPoolID > (((int)poolVector_->size()) - 1))
   {
      TRACELOG(ERRORLOG, OPMLOG, "Invalid object poolID",0,0,0,0,0,0);
      return ERROR;
   }//end if

   //get the correct Pool from the OPM
   ObjectPool* pool = (*poolVector_)[objectPoolID];

   poolVectorMutex_.release();

   //check to make sure the Pool has been stored in the OPM for this object
   //generate an ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Object Pool ID %d does not exist in OPM", objectPoolID,0,0,0,0,0);
      return ERROR;
   }//end if
    
   //get the byte array size from the pool
   return pool->getObjectInitParam();
}//end getPoolObjectInitParam


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method prints the usage summary for all Object Pools being
//              managed within the OPM. This method is called by default when
//              the shutdown method is called.
// Design:     
//-----------------------------------------------------------------------------
void OPM::printAllPoolsSummary()
{
   poolVectorMutex_.acquire();
   //print the total number of pools being managed by the OPM
   // (use %zd for printing size_t types)
   printf("Total Pools managed by OPM: %zd\n", poolVector_->size());
    
   //traverse the OPM vector and call printUsageSummary on each pool
   for (unsigned int i = 0; i < poolVector_->size(); i++)
   {
      (*poolVector_)[i]->printUsageSummary();
   }//end for
   poolVectorMutex_.release();
}//end printAllPoolsSummary


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method prints the usage summary for a particular Object Pool. 
// Design:     
//-----------------------------------------------------------------------------
void OPM::printPoolSummary(int objectPoolID)
{
   poolVectorMutex_.acquire();
   //check the poolID
   if (objectPoolID > (((int)poolVector_->size()) - 1))
   {
      TRACELOG(ERRORLOG, OPMLOG, "Invalid object poolID",0,0,0,0,0,0);
      return;
   }//end if

   //get the correct Pool from the OPM
   ObjectPool* pool = (*poolVector_)[objectPoolID];

   poolVectorMutex_.release();
    
   //check to make sure the Pool has been stored in the OPM for this object
   //generate a ERROR msg if the get operation returns 'NULL'
   if (pool == NULL)
   {
      TRACELOG(ERRORLOG, OPMLOG, "Object Pool ID %d does not exist in OPM", objectPoolID,0,0,0,0,0);
      return;
   }//end if
    
   pool->printUsageSummary();
}//end printPoolSummary


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Method tests all of the pools and prints the statistics per 
//              Object Pool for objects which HAVE NOT been properly 
//              returned/released into the OPM.
// Design:     
//-----------------------------------------------------------------------------
void OPM::checkForUnreleasedObjects()
{
   //print that we are checking for unreleased objects
   TRACELOG(DEBUGLOG, OPMLOG, "Checking for Objects NOT properly released into OPM",0,0,0,0,0,0);
      
   poolVectorMutex_.acquire();

   //traverse the OPM vector and check for unreleased objects in the Used Pool
   for (unsigned int i = 0; i < poolVector_->size(); i++)
   {
      ObjectPool* pool = (*poolVector_)[i];
      if (pool->getCurrentUsedObjects() > 0)
      {
         ostringstream ostr;
         ostr <<  "Pool (" << pool->getObjectType() << ") with Current Capacity = " << pool->getCurrentCapacity() << " has " << pool->getCurrentUsedObjects() << " Unreleased objects" << ends;
         STRACELOG(ERRORLOG, OPMLOG, ostr.str().c_str());
      }//end if
   }//end for

   poolVectorMutex_.release();
}//end checkForUnreleasedObjects

//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------
