/******************************************************************************
*
* File name:   MailboxProcessor.cpp
* Subsystem:   Platform Services
* Description: This class reads the messages from the Mailbox message queue
*              and calls the appropriate Message Handler. By default, the
*              mailbox processor remains blocked on the mailbox queue waiting
*              for messages.
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

#include "MailboxProcessor.h"
#include "MailboxOwnerHandle.h"
#include "MessageHandlerList.h"

#include "platform/logger/Logger.h"

#include "platform/common/Defines.h"

#include "platform/threadmgr/ThreadManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Private static instance pointer to the MailboxProcessor
MailboxProcessor::MailboxProcessor* MailboxProcessor::mailboxProcessor_ = NULL;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxProcessor::MailboxProcessor(MessageHandlerList* handlerList, 
                                   MailboxOwnerHandle& mailboxOwnerHandle)
                                   : handlerList_(handlerList),
                                     mailboxOwnerHandle_(mailboxOwnerHandle)
{
   // Populate the static instance member
   mailboxProcessor_ = this;
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxProcessor::~MailboxProcessor()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reads messages from the mailbox message queue and calls the
//              appropriate message handler. If the queue is empty it will block.
//              After the message has been processed it will be deleted (or
//              it will be checked in to the OPM if it is an OPM managed object).
//              So applications SHOULD NOT DELETE messages when they handle them.
//              If the mailbox has not be activated by the application, then
//              getMessage() will return NULL, and this method will return.
// Design:
//-----------------------------------------------------------------------------
void MailboxProcessor::processMailbox(int numberThreads)
{
   // Check the number of Threads passed
   if (numberThreads <= 0)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Illegal number of threads (%d) passed to processMailbox",
         numberThreads,0,0,0,0,0);
      return;
   }//end if

   // If the number of requested threads is the default (1), then we simply call
   // processMailboxInternal method from this thread context (which blocks).
   if (numberThreads == 1)
   {
      processMailboxInternal();
   }//end if 
   // Else if the requested threads is N, and N is greater than 1, we spawn N 
   // threads to perform processMailboxInternal, and we block the current thread
   else
   {
      // Spawn N threads for processing - use default (non-realtime) scheduling
      ThreadManager::createThreadPool( numberThreads, (ACE_THR_FUNC)MailboxProcessor::invokeStatic,
         (void*) 0, "MailboxProcessor", true);   

      // Here, instead of blocking the current thread, we could spawn N-1 threads and 
      // let the current thread context execute 'processMailboxInternal()' as well. The
      // advantage to this is that we do not have to rely on 'wait' and we have 1 less thread.
      // The disadvantage is that we loose the ThreadManager's restart and death handler
      // protection for this current thread context.

      // Block here so the child threads can do their work
      ACE_Thread_Manager::instance()->wait();
   }//end else
}//end processMailbox


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Loop on the Mailbox message queue processing messages
// Design:
//-----------------------------------------------------------------------------
void MailboxProcessor::processMailboxInternal()
{
   MessageBase* messagePtr = mailboxOwnerHandle_.getMessage();
   while ( messagePtr != NULL )
   {
      if (handlerList_)
      {
         // Get the mapping between the message handler functor and the message Id
         const MessageHandler& messageHandler = handlerList_->find(messagePtr->getMessageId());
         messageHandler(messagePtr);
      }//end if

      messagePtr->deleteMessage();
      messagePtr = NULL;
      messagePtr = mailboxOwnerHandle_.getMessage();
   }//end while
}//end processMailboxInternal


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Static method invocation of processMailboxInternal
// Design:
//-----------------------------------------------------------------------------
void MailboxProcessor::invokeStatic()
{
   if (mailboxProcessor_)
   {
      mailboxProcessor_->processMailboxInternal();
   }//end if
   else
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Mailbox Processor Instance is NULL",0,0,0,0,0,0);
   }//end else
}//end invokeStatic


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

