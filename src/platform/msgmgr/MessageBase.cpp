/******************************************************************************
*
* File name:   MessageBase.cpp
* Subsystem:   Platform Services
* Description: This is the base class for all messages. This is an abstract
*              class and cannot be instantiated. All messages in the system
*              will be derived from this base class.
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

#include "MessageBase.h"
#include "MessageBuffer.h"
#include "MailboxAddress.h"

#include "platform/logger/Logger.h"
#include "platform/common/Defines.h"
#include "platform/common/MessageIds.h"

#include "platform/opm/OPM.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
unsigned short MessageBase::getStaticMessageId()
{
   return MSGMGR_BASE_ID;  // Defined in MessageIds.h
}//end getStaticMessageId


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
unsigned char MessageBase::getDestinationMailboxId() const
{
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call of base class getDestinationMailboxId for messageId %d",
      getMessageId(),0,0,0,0,0);
   // return FF for the default mailbox id
   return 0xff;
}//end getDestinationMailboxId


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: Accepts the source address of the task/thread that originated
//              the message, and the source and destination context ids. The
//              context identifiers are optional and are used only when messages
//              are part of a context or transaction. The source context id refers
//              to the context in the task that creates the message, and the 
//              destination context id refers to a context in the task that
//              will receive this message. 
// Design:     
//-----------------------------------------------------------------------------
MessageBase::MessageBase(const MailboxAddress& sourceAddress,
                         const unsigned int versionNumber,
                         const unsigned int sourceContextId,
                         const unsigned int destinationContextId)
                         : sourceAddress_(sourceAddress),
                           sourceContextId_(sourceContextId),
                           destinationContextId_(destinationContextId),
                           versionNumber_(versionNumber),
                           isReusable_(false),
                           priorityLevel_(0)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageBase::~MessageBase()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Provide a default serialization implementation
// Design:      returns OK or ERROR if not successful 
//-----------------------------------------------------------------------------
int MessageBase::serialize(MessageBuffer& buffer)
{
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call of base class serialize for messageId %d",
      getMessageId(),0,0,0,0,0);
   buffer << this->getMessageId();
   return OK;
}//end serialize


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Default deserialization implementation -- for developer
//              reference ONLY -- developer must provide a subclassed implementation
// Design:      
//-----------------------------------------------------------------------------
MessageBase* MessageBase::deserialize(MessageBuffer* buffer)
{
   buffer = NULL;
   TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal call of base class static deserialize bootstrap method",0,0,0,0,0,0);
   return NULL;
}//end deserialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns a copy of the source context id for this message 
// Design:
//-----------------------------------------------------------------------------
unsigned int MessageBase::getSourceContextId() const
{
   return sourceContextId_;
}//end getSourceContextId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns a copy of the destination context id for this message 
// Design:
//-----------------------------------------------------------------------------
unsigned int MessageBase::getDestinationContextId() const
{
   return destinationContextId_;
}//end getDestinationContextId


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns a reference to the source address - the name of the task
//              that created this message.
// Design:      This is a constant member function and it should never modify the
//              message for which it is called.
//-----------------------------------------------------------------------------
const MailboxAddress& MessageBase::getSourceAddress() const
{
   return sourceAddress_;
}//end getSourceAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform self-deletion
// Design:
//-----------------------------------------------------------------------------
void MessageBase::deleteMessage()
{
   // Check to see if this object is OPM poolable (inherits from OPMBase) and if 
   // it was created by the OPM (it may not have been); if so, check the object back into the OPM
   if (isPoolable() && OPM::isCreatedByOPM((OPMBase*)this))
   {
      // Macro to release object back into the OPM where it gets cleaned.
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Delete message is deferring to OPM release",0,0,0,0,0,0);
      OPM_RELEASE((OPMBase*)this); 
   }//end if
   // Check to see if this object is reusable. If so, it is the responsibility of the 
   // application developer to delete it. Or, if the developer marks it as no longer reusable,
   // then it can get deleted below (the next time through the MsgMgr framework).
   else if (isReusable_)
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Delete message is deferred for Reusable message",0,0,0,0,0,0);
      return; 
   }//end else if
   // If the message is NOT pooled inside the OPM, and its NOT Reusable, then we delete it here
   // (regardless of whether or not the message inherits from OPMBase)
   else
   {
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Delete message is called by MsgMgr framework",0,0,0,0,0,0);
      delete this;
   }//end else
}//end deleteMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Flag this message as high priority. 
// Design: This flag will cause the message to be re-ordered in the Mailbox 
//         message queue (so will NOT follow FIFO rules). Default priority
//         is 0. Priorities greater than 0 have greater priority.
//-----------------------------------------------------------------------------
void MessageBase::setPriority(unsigned int priorityLevel)
{
   priorityLevel_ = priorityLevel;
}//end setHighPriority


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Re-apply the version number to this message
// Design:
//-----------------------------------------------------------------------------
void MessageBase::setVersion(unsigned int versionNumber)
{
   versionNumber_ = versionNumber;
}//end setVersion


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve the priority level for this message.
// Design: Default priority is 0. Priorities greater than 0 have greater priority.
//-----------------------------------------------------------------------------
unsigned int MessageBase::getPriority()
{
   return priorityLevel_;
}//end getPriority


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve the version Number for this message.
// Design:
//-----------------------------------------------------------------------------
unsigned int MessageBase::getVersion()
{
   return versionNumber_;
}//end getVersion


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return whether or not this object is managed by an OPM pool
// Design:
//-----------------------------------------------------------------------------
bool MessageBase::isPoolable()
{
   return ObjectBase::isPoolable();
}//end isPoolable


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return whether or not this object is reusable
// Design:
//-----------------------------------------------------------------------------
bool MessageBase::isReusable()
{
   return isReusable_;
}//end isReusable


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

