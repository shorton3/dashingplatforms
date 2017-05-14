/******************************************************************************
*
* File name:   MailboxHandle.cpp
* Subsystem:   Platform Services
* Description: This class is a handle to a mailbox. The mailbox itself may be
*              of local or distributed type. The mailbox handles prevent
*              application developers from getting direct references
*              to the mailboxes themselves.
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

#include "MailboxHandle.h"
#include "MailboxBase.h"

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
MailboxHandle::MailboxHandle(MailboxBase* mailboxPtr)
     :mailboxPtr_(mailboxPtr)
{
   // Check the mailbox pointer.
   if (mailboxPtr_)
   {
      mailboxPtr_->acquire();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Attempt to create MailboxHandle with NULL/invalid mailbox pointer",0,0,0,0,0,0);
      // Abort the process (SHOULD NEVER GET HERE!)
      exit(ERROR);
   }//end else
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxHandle::~MailboxHandle()
{
   TRACELOG(DEVELOPERLOG, MSGMGRLOG, "MailboxHandle destructor/release called",0,0,0,0,0,0);
   mailboxPtr_->release();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Calls the post() method on the actual mailbox. Returns ERROR if
//              the handle has not been acquired else returns the error code
//              returned by the mailbox post() method.
// Design:
//-----------------------------------------------------------------------------
int MailboxHandle::post(MessageBase* messagePtr, const ACE_Time_Value* timeout)
{
   return (mailboxPtr_->post(messagePtr, timeout));
}//end post


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve the mailbox debug flag
// Design:
//-----------------------------------------------------------------------------
int MailboxHandle::getDebugValue()
{
   return mailboxPtr_->getDebugValue();
}//end getDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Set the mailbox debug flag
// Design:
//-----------------------------------------------------------------------------
void MailboxHandle::setDebugValue(int debugValue)
{
   mailboxPtr_->setDebugValue(debugValue);
}//end setDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return whether or not this Mailbox is a remote proxy or not
// Design:
//-----------------------------------------------------------------------------
bool MailboxHandle::isProxy()
{
   return mailboxPtr_->isProxy();
}//end isProxy


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& MailboxHandle::getMailboxAddress()
{
   return mailboxPtr_->getMailboxAddress();
}//end getMailboxAddress


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the sent message counter value
// Design:
//-----------------------------------------------------------------------------
unsigned int MailboxHandle::getSentCount()
{
   return mailboxPtr_->getSentCount();
}//end getSentCount


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MailboxHandle::toString()
{
   string s = "";
   return (s);
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

