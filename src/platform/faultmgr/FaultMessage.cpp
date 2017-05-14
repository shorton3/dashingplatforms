/******************************************************************************
*
* File name:   FaultMessage.cpp
* Subsystem:   Platform Services
* Description: Fault Message class encapsulates the information for a single
*              alarm so that it can be enqueued by the originating application
*              and then dequeued by the Fault Manager process for transfer to the
*              EMS.
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

#include <cstdio>
#include <cstring>
#include <sstream>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "FaultMessage.h"

#include "platform/common/Defines.h"

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
FaultMessage::FaultMessage()
           : managedObject(LAST_MANAGED_OBJECT),
             managedObjectInstance(0),
             alarmCode(ALARM_OS_RESOURCE_BASE),
             eventCode(EVENT_REPORT_LAST),
             alarmSeverity(ALARM_LAST_SEVERITY),
             pid(0),
             timeStamp(0)
{
   ACE_OS::strncpy(neid, "000000000", NEID_LENGTH);
   neidPI = neid;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Copy Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
FaultMessage::FaultMessage( const FaultMessage& rhs )
{
   ACE_OS::strncpy(neid, rhs.neidPI, NEID_LENGTH);
   neidPI = neid;

   managedObject = rhs.managedObject;
   managedObjectInstance = rhs.managedObjectInstance;
   alarmCode = rhs.alarmCode;
   eventCode = rhs.eventCode;
   alarmSeverity = rhs.alarmSeverity;
   pid = rhs.pid;
   timeStamp = rhs.timeStamp;
}//end Copy Constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
FaultMessage::~FaultMessage()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reset the data members
// Design:
//-----------------------------------------------------------------------------
void FaultMessage::reset()
{
   ACE_OS::strncpy(neid, "000000000", NEID_LENGTH);
   neidPI = neid;

   managedObject = LAST_MANAGED_OBJECT;
   managedObjectInstance = 0;
   alarmCode = ALARM_OS_RESOURCE_BASE;
   eventCode = EVENT_REPORT_LAST;
   alarmSeverity = ALARM_LAST_SEVERITY;
   pid = 0;
   timeStamp = 0;
}//end reset


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded assignment operator
// Design:
//-----------------------------------------------------------------------------
FaultMessage& FaultMessage::operator= ( const FaultMessage& rhs )
{
   if (this != &rhs)
   {
      ACE_OS::strncpy(neid, rhs.neidPI, NEID_LENGTH);
      neidPI = neid;

      managedObject = rhs.managedObject;
      managedObjectInstance = rhs.managedObjectInstance;
      alarmCode = rhs.alarmCode;
      eventCode = rhs.eventCode;
      alarmSeverity = rhs.alarmSeverity;
      pid = rhs.pid;
      timeStamp = rhs.timeStamp;
   }//end if
   return *this;
}//end assignment operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Equality Operator
// Design:
//-----------------------------------------------------------------------------
bool FaultMessage::operator== (const FaultMessage& rhs) const
{
   if ( (neidPI == rhs.neidPI) &&
        (managedObject == rhs.managedObject) &&
        (managedObjectInstance == rhs.managedObjectInstance) &&
        (alarmCode == rhs.alarmCode) &&
        (eventCode == rhs.eventCode) &&
        (alarmSeverity == rhs.alarmSeverity) &&
        (pid == rhs.pid) &&
        (timeStamp == rhs.timeStamp) )
   {
      return true;
   }//end if
   else
   {
      return false;
   }//end else
}//end equality operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Comparison Operator
// Design:
//-----------------------------------------------------------------------------
bool FaultMessage::operator< (const FaultMessage& rhs) const
{
   ostringstream indexingStream;
   indexingStream << neidPI << managedObject << managedObjectInstance  
                  << alarmCode << eventCode << alarmSeverity << pid << timeStamp << ends;

   ostringstream rhsIndexingStream;
   rhsIndexingStream << rhs.neidPI << rhs.managedObject << rhs.managedObjectInstance
                     << rhs.alarmCode << rhs.eventCode << rhs.alarmSeverity
                     << rhs.pid << rhs.timeStamp << ends;
                                                                                                                                      
   bool result = (indexingStream.str() < rhsIndexingStream.str());
   return result;
}//end comparison operator


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

