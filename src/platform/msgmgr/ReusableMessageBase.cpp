/******************************************************************************
*
* File name:   ReusableMessageBase.cpp
* Subsystem:   Platform Services
* Description: Inherits from the MessageBase to provide for Messages that are
*              reusable/resendable without the MsgMgr framework consuming them;
*              that is, they will not be consumed when the MsgMgr is done
*              processing them (so they can be sent over and over again).
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

#include "ReusableMessageBase.h"

#include "platform/common/Defines.h"

#include "platform/logger/Logger.h"

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
ReusableMessageBase::ReusableMessageBase(const MailboxAddress& sourceAddress,
                      unsigned int versionNumber,
                      unsigned int sourceContextId,
                      unsigned int destinationContextId)
                     :MessageBase(sourceAddress, versionNumber, sourceContextId, destinationContextId),
                      hasBeenProcessed_(false)
{
   // Mark this message as reusable in the base class
   isReusable_ = true;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
ReusableMessageBase::~ReusableMessageBase()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Mutator method to set whether or not the Message has already been
//    processed this time around
// Design:     
//-----------------------------------------------------------------------------
void ReusableMessageBase::setHasBeenProcessed(bool hasBeenProcessed)
{
   hasBeenProcessed_ = hasBeenProcessed;
}//end setHasBeenProcessed


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Accessor method used to determine if the Message has already
//    been processed this time around
// Design:
//-----------------------------------------------------------------------------
bool ReusableMessageBase::hasBeenProcessed()
{
   return hasBeenProcessed_;
}//end hasBeenProcessed

                                                                                                                             
//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Mark this Message as no longer reusable so that it can be consumed
//    by the MsgMgr framework
//-----------------------------------------------------------------------------
void ReusableMessageBase::setNoLongerReusable()
{
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Application is setting message to be no longer reusable",0,0,0,0,0,0);
   // Mark this message as no longer reusable in the base class
   isReusable_ = false;
}//end setNoLongerReusable


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

