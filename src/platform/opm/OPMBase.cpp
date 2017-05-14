/******************************************************************************
*
* File name:   OPMBase.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sstream>

#include <ace/OS_NS_unistd.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "OPMBase.h"

// For logging
#include "platform/logger/Logger.h"

//#include "platform/utilities/DebugUtils.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
OPMBase::OPMBase()
  :next_(NULL),
   prev_(NULL),
   objectType_("<Unassigned>"),
   poolID_(UNKNOWN_POOLID)
{
   //Identify this ObjectBase as a poolable object so that we know in child class
   // instantiations that it inherits from OPMBase
   setPoolable(true);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
OPMBase::~OPMBase()
{
   ostringstream ostr;
   ostr << "WARNING! Possible illegal deletion of OPM object of type ("
        << objectType_ << ") if owned/created by OPM" << ends;
   STRACELOG(DEVELOPERLOG, OPMLOG, ostr.str().c_str());

   // If we need to debug who deleted it
   //DebugUtils::printStackTrace();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return whether or not there is a next node in the Linked List
// Design:     
//-----------------------------------------------------------------------------
bool OPMBase::isNextNull()
{
   return (next_ == NULL);
}//end isNextNull


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return whether or not there is a previous node in the Linked List
// Design:     
//-----------------------------------------------------------------------------
bool OPMBase::isPrevNull()
{
   return (prev_ == NULL);
}//end isPrevNull


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the next OPMBase object referred to in the Linked List
// Design:     
//-----------------------------------------------------------------------------
OPMBase* OPMBase::getNext()
{
   return next_;
}//end getNext


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the next OPMBase object referred to in the Linked List
// Design:     
//-----------------------------------------------------------------------------
void OPMBase::setNext(OPMBase* newObject)
{
   next_ = newObject;
}//end setNext


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the previous OPMBase object referred to in the Linked List
// Design:     
//-----------------------------------------------------------------------------
OPMBase* OPMBase::getPrev()
{
   return prev_;
}//end getPrev


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the previous OPMBase object referred to in the Linked List
// Design:     
//-----------------------------------------------------------------------------
void OPMBase::setPrev(OPMBase* newObject)
{
   prev_ = newObject;
}//end setPrev

        
//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the integer poolID that identifies which pool this object belongs to
// Design:     
//-----------------------------------------------------------------------------
int OPMBase::getPoolID()
{
   return poolID_;
}//end getPoolID


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the integer poolID that identifies which pool this object belongs to
// Design:     
//-----------------------------------------------------------------------------
void OPMBase::setPoolID(int poolID)
{
   poolID_ = poolID;
}//end setPoolID


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return a pointer to the object type string for this object
// Design:     
//-----------------------------------------------------------------------------
const char* OPMBase::getObjectTypeStr()
{
   return objectType_;
}//end getObjectTypeStr

   
//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the object type string for this object - called upon object creation
// Design:     
//-----------------------------------------------------------------------------
void OPMBase::setObjectTypeStr(const char* objectType)
{
   objectType_ = objectType;
}//end setObjectTypeStr
 
     
//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: String'ized debugging tool
// Design:     
//-----------------------------------------------------------------------------
string OPMBase::toString()
{
   char buff[50];

   // Need to test if we are 64 bit or 32 bits. ace/Basic_Types.h is incorrect
   // since it bases ACE_SIZEOF_INT on the value of INT_MAX from limits.h in
   // Redhat. But, INT_MAX is always 4; whereas LONG_MAX changes for 32 and 64
   // bit variants. Within /usr/include/limits.h, we see that Redhat's test is
   // to check #if __WORDSIZE==64, so we'll do that too.

   #if __WORDSIZE == 64
     sprintf(buff, "Pool[%d]-%s(%lx)", poolID_, objectType_, (long)this);
   #else
     sprintf(buff, "Pool[%d]-%s(%x)", poolID_, objectType_, (int)this);
   #endif
   return buff;
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

