/******************************************************************************
*
* File name:   LocalMailbox.cpp
* Subsystem:   Platform Services
* Description: LocalMailbox is a subclass of MailboxBase.  It is used solely for
*              local messages and does not support receipt of messages from
*              remote nodes in the system. This class may be subclassed in
*              order to derive more complex Mailbox implementations. 
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

#include <ace/Time_Value.h>
#include <ace/Message_Block.h>
#include <ace/Select_Reactor.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageBase.h"
#include "LocalMailbox.h"
#include "MailboxAddress.h"
#include "MailboxLookupService.h"
#include "MailboxOwnerHandle.h"
#include "MessageBlockWrapper.h"

#include "platform/common/MailboxNames.h"

#include "platform/opm/OPM.h"

#include "platform/logger/Logger.h"

#include "platform/threadmgr/ThreadManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

/* From the C++ FAQ, create a module-level identification string using a compile
   define - BUILD_LABEL must have NO spaces passed in from the make command
   line */
#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused)) = "@(#)MsgMgr Framework"
   "\n   Build Label: " XstrConvert(BUILD_LABEL)
   "\n   Compile Time: " __DATE__ " " __TIME__;

#define QUEUE_THRESHOLD 100

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LocalMailbox::LocalMailbox(const MailboxAddress& localAddress)
  :localAddress_(localAddress),
   messageBlockWrapperPoolID_(-1)
{
   // Create a (Thread Safe) pool of Message Block Wrapper objects to be used inside the 
   // ACE_Message_Queue
   messageBlockWrapperPoolID_ = OPM::createPool("MessageBlockWrapper", 0, 
      (OPM_INIT_PTR)&MessageBlockWrapper::initialize, 0.8, 50, 100, true, OPM_GROWTH_ALLOWED);
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: Attempt to dispose of all of the messages and their contents
//              that are still in the message queue
// Design:     
//-----------------------------------------------------------------------------
LocalMailbox::~LocalMailbox()
{
   // Flag that we are shutting down
   isShuttingDown_ = TRUE;
   
   selectReactor_->end_reactor_event_loop(); 

   ACE_Message_Block* messageBlock = NULL;
   while (!this->messageQueue_.is_empty())
   {
      if ( messageQueue_.dequeue(messageBlock) == -1)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error dequeuing from message queue",0,0,0,0,0,0);
      }//end if

      // Cast the ACE_Message_Block back into our data type
      char* c_data = messageBlock->base();

      // Instead of releasing the Message_Block so that it can be reference counted and disposed of
      // we just check the message block back into the OPM. Here, we must first cast to
      // MessageBlockWrapper*; otherwise, the contained ObjectPoolId needed by OPM to determine
      // which pool to release into will not be visible!!
      OPM_RELEASE((OPMBase*)((MessageBlockWrapper*)messageBlock));
      if (c_data)
      {
         void* v_data = (void*) c_data;
         MessageBase* messagePtr = (MessageBase*) v_data;
         if (messagePtr->isPoolable())
         {
            // Release it back into its OPM pool
            OPM_RELEASE((OPMBase*)messagePtr); 
         }//end if
         else
         {
            messagePtr->deleteMessage();
         }//end else
      }//end if
   }//end while
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: This method starts the reactor thread needed for processing
//              timer messages, etc.
// Design:
//-----------------------------------------------------------------------------
void LocalMailbox::startReactor(void* arg)
{
   // Convert the void* arg to a Reactor pointer
   ACE_Reactor* reactor = static_cast<ACE_Reactor*> (arg);

   // Set Reactor thread ownership
   reactor->owner (ACE_Thread::self ());
   // Start the reactor processing loop
   while ((isShuttingDown_ == FALSE) && (reactor->reactor_event_loop_done () == 0))
   {
      int result = reactor->run_reactor_event_loop ();

      char errorBuff[200];
      char* resultStr = strerror_r(errno, errorBuff, strlen(errorBuff));
      if (resultStr == NULL)
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Error getting errno string for (%d)",errno,0,0,0,0,0);
      }//end if
      ostringstream ostr;
      ostr << "Reactor event loop returned with code (" << result << ") and errno (" << resultStr << ")" << ends;
      STRACELOG(ERRORLOG, MSGMGRLOG, ostr.str().c_str());

      // Perform reset on the reactor
      reactor->reset_reactor_event_loop();
   }//end while

   // Need to do something to keep ThreadManager from restarting it
   //if (isShuttingDown_)
   //{
   //   ACE_Thread_Manager::instance()->at_exit(NULL,0);
   //}//end if
}//end startReactor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: This method activates the mailbox and registers it with
//              the mailbox lookup service. It also creates the message queue.
// Design:
//-----------------------------------------------------------------------------
int LocalMailbox::activate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (isActive())
   {
      return OK;
   }//end if

   TRACELOG(DEBUGLOG, MSGMGRLOG, "Local Mailbox activate is called",0,0,0,0,0,0);

   // Activate the ACE Message Queue
   messageQueue_.activate();
   MailboxLookupService::registerMailbox(mailboxOwnerHandle, this);
   setActive(TRUE);

   return OK;
}//end activate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: This method deactivates the mailbox and deregisters it with
//              the mailbox lookup service. 
// Design: 
//-----------------------------------------------------------------------------
int LocalMailbox::deactivate(MailboxOwnerHandle* mailboxOwnerHandle)
{
   if (!isActive())
   {
      return OK;
   }//end if

   TRACELOG(DEBUGLOG, MSGMGRLOG, "Local Mailbox deactivate is called",0,0,0,0,0,0);
 
   // Deactivate the ACE Message Queue
   messageQueue_.deactivate();
                                                                                          
   setActive(FALSE);
   MailboxLookupService::deregisterMailbox(mailboxOwnerHandle);
   return OK;
}//end deactivate


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns ERROR for an error, OK otherwise.
// Design:      
//-----------------------------------------------------------------------------
int LocalMailbox::post(MessageBase* messagePtr, const ACE_Time_Value* timeout)
{
   int result;

   if (!isActive() || (messagePtr == NULL))
   {
      return ERROR;
   }//end if

   if (debugValue_)
   {
      ostringstream debugMsg;
      debugMsg << "##POSTING MESSAGE## "
               << " SOURCE_ADDRESS>> " << ((MailboxAddress)messagePtr->getSourceAddress()).toString()
               << " DESTINATION_ADDRESS>> " << localAddress_.toString()
               << " MESSAGE_ID>> 0x" << hex << messagePtr->getMessageId()
               << " MESSAGE_CONTENT>> " << messagePtr->toString()
               << ends;
      STRACELOG(DEBUGLOG, MSGMGRLOG, debugMsg.str().c_str()); 
   }//end if

   // Since all messages must travel through the ACE_Message_Queue inside of an 
   // ACE_Message_Block wrapper object, we retrieve a Message_Block from the OPM pool
   // where we created some. Store a pointer to our local message inside the Message_Block
   // (Message_Block needs a char*, so we need to do some manipulation). First cast to a 
   // void pointer (this is dangerous, so need to keep close eye on it). Then cast to char*.
   void* v_data = (void *) messagePtr;
   char* c_data = (char *) v_data;

   // Do lots of casting to work between these base classes
   MessageBlockWrapper* messageBlockWrapper = (MessageBlockWrapper*)OPM_RESERVE(messageBlockWrapperPoolID_);
   if (!messageBlockWrapper)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "OPM returned null message block wrapper",0,0,0,0,0,0);
      return ERROR;
   }//end if
   ACE_Message_Block* messageBlock = (ACE_Message_Block*)messageBlockWrapper;

   // Store our manipulated message pointer inside the ACE Message Block 
   messageBlock->base(c_data, sizeof(c_data));

   // Set the messageBlock priority that it will carry with it through the Message Queue
   // as specified in the posted message (MessageBase). NOTE that ACE maintains FIFO
   // order when messages of the same priority are inserted consecutively. (ace/Message_Queue_T.h)
   messageBlock->msg_priority (ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY + (int)messagePtr->getPriority());

   // Enqueue the message to the mailbox priority enabled queue (in case the 
   // developer set the priority flag for this message)
   if ( (result = messageQueue_.enqueue_prio(messageBlock, (ACE_Time_Value*)timeout)) == -1)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Failed to enqueue message",0,0,0,0,0,0);
      return ERROR;
   }//end if

   // Wake up any sleeping dequeue threads on getMessage
   manualEvent_.signal();

   // Check the capacity of the message queue
   if (result > QUEUE_THRESHOLD)
   {
      TRACELOG(WARNINGLOG, MSGMGRLOG, "Mailbox queue over threshold with count %d", result,0,0,0,0,0);
   }//end if

   return OK;
}//end post


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Block until a message is available in the queue.
//              If the mailbox is deactivated this method may return NULL.
// Design:      If the queue has messages, it dequeues from the head and returns
//              the message. If no messages are in the queue, this method will
//              block waiting on a mutex that gets released when a message is
//              inserted in the queue.
//-----------------------------------------------------------------------------
MessageBase* LocalMailbox::getMessage(unsigned short timeout)
{
   MessageBase* messagePtr = NULL;

   // For now, use a dummy variable to prevent unused variable compiler warning
   unsigned short tmp __attribute__ ((unused)) = timeout;

   if (!isActive())
   {
      return NULL;
   }//end if

   // Loop performing periodic sleeps until the queue becomes non-Empty
   while (messageQueue_.is_empty())
   {
      // sleep for 50 msec
      //usleep(50000);

      // Instead of doing a busy wait, let's do a real wait until we are signaled
      // by a post event that the queue is no longer empty
      manualEvent_.wait();
      // When wait() completes, we need to manually reset the Event so that wait()
      // will again block the next time the queue is empty
      manualEvent_.reset();
   }//end while

   // Retrieve the message that is now available. First check to make sure the
   // mailbox is still active
   if (!isActive())
   {
      return NULL;
   }//end if 

   // Since all messages must travel through the ACE_Message_Queue inside of an
   // ACE_Message_Block wrapper object, we must retrieve the Message_Block from the queue
   // and then extract the MessageBase* object from the char* that Message Block needs.
   ACE_Message_Block* messageBlock = NULL;

   // Retrieve from the queue
   if ( (messageQueue_.dequeue_head(messageBlock) ) == -1)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Error dequeuing from mailbox queue",0,0,0,0,0,0);
      return NULL;
   }//end if

   // Cast the ACE_Message_Block back into our data type
   char* c_data = messageBlock->base();

   // Instead of releasing the Message_Block so that it can be reference counted and disposed of
   // we just check the message block back into the OPM. Here, we must first cast to 
   // MessageBlockWrapper*; otherwise, the contained ObjectPoolId needed by OPM to determine
   // which pool to release into will not be visible!!
   OPM_RELEASE((OPMBase*)((MessageBlockWrapper*)messageBlock)); 
   if (c_data)
   {
      void* v_data = (void*) c_data;
      messagePtr = (MessageBase*) v_data;
   }//end if
   else 
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Dequeued a Message Block containing a NULL data block",0,0,0,0,0,0);
      return NULL;
   }//end else

   // Return the message
   return messagePtr;
}//end getMessage


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Do not block if a message is not available in the queue; return
//              a NULL. If the mailbox is deactivated this method may return NULL.
// Design:      If the queue has messages, it dequeues from the head and returns
//              the message. 
//-----------------------------------------------------------------------------
MessageBase* LocalMailbox::getMessageNonBlocking()
{
   MessageBase* messagePtr = NULL;

   if (!isActive())
   {
      return NULL;
   }//end if

   // Since all messages must travel through the ACE_Message_Queue inside of an
   // ACE_Message_Block wrapper object, we must retrieve the Message_Block from the queue
   // and then extract the MessageBase* object from the char* that Message Block needs.
   ACE_Message_Block* messageBlock = NULL;

   if (messageQueue_.is_empty())
   {
      return NULL;
   }//end if
   else if ( (messageQueue_.dequeue_head(messageBlock) ) == -1)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Error dequeuing from non-blocking mailbox queue",0,0,0,0,0,0);
      return NULL;
   }//end else if

   // Cast the ACE_Message_Block back into our data type
   char* c_data = messageBlock->base();
   // Instead of releasing the Message_Block so that it can be reference counted and disposed of
   // we just check the message block back into the OPM. Here, we must first cast to
   // MessageBlockWrapper*; otherwise, the contained ObjectPoolId needed by OPM to determine
   // which pool to release into will not be visible!!
   OPM_RELEASE((OPMBase*)((MessageBlockWrapper*)messageBlock)); 
   if (c_data)
   {
      void* v_data = (void*) c_data;
      messagePtr = (MessageBase*) v_data;
   }//end if
   else
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Dequeued a Message Block containing a NULL data block",0,0,0,0,0,0);
      return NULL;
   }//end else

   // Return the message
   return messagePtr;   
}//getMessageNonBlocking


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Allow applications to create a local mailbox without giving them
//              direct access to the mailbox itself.
// Design: 
//-----------------------------------------------------------------------------
MailboxOwnerHandle* LocalMailbox::createMailbox(const MailboxAddress& localAddress)
{
   // Enforce the rule that Local Mailbox Address can ONLY consist of LocationType
   // and MailboxName
   if ( (localAddress.mailboxType != UNKNOWN_MAILBOX_TYPE) ||
        (localAddress.locationType != LOCAL_MAILBOX) ||
        (localAddress.shelfNumber != UNKNOWN_SHELF_NUMBER) ||
        (localAddress.slotNumber != UNKNOWN_SLOT_NUMBER) ||
        (strcmp(localAddress.mailboxName.c_str(), UNKNOWN_MAILBOX_NAME) == 0) ||
        (strcmp(localAddress.neid.c_str(), UNKNOWN_NEID) != 0) ||
        (localAddress.inetAddress != MailboxAddress::defaultUnknownInetAddress_) ||
        (localAddress.redundantRole != REDUNDANT_UNKNOWN))
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Invalid LocalMailbox Address. Should only consist of LocationType and MailboxName",0,0,0,0,0,0);
      return NULL;
   }//end if

   // Instantiate the Mailbox and Create a new ACE_Select_Reactor for timers to use
   LocalMailbox* mailboxPtr = new LocalMailbox(localAddress);
   if (!mailboxPtr)
   {
      return NULL;
   }//end if
   else
   {
      mailboxPtr->selectReactor_ = new ACE_Reactor (new ACE_Select_Reactor, 1);
   }//end else

   // Create an owner handle and return it
   MailboxOwnerHandle* mailboxOwnerHandlePtr = new MailboxOwnerHandle(mailboxPtr);
   if (!mailboxOwnerHandlePtr)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Unable to create an owner handle to Local Mailbox",0,0,0,0,0,0);
   }//end if

   // Activate the mailbox
   mailboxPtr->activate(mailboxOwnerHandlePtr);

   // Start the Reactor Event Loop -- used for Timer processing. This has to be
   // started in a separate thread. Here, returns OS assigned unique thread Id
   ThreadManager::createThread((ACE_THR_FUNC)LocalMailbox::startReactor,
      (void*)mailboxPtr->selectReactor_, "LocalMailboxReactor", true);
 
   return mailboxOwnerHandlePtr;
}//end createMailbox


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the post timeout
// Design:
//-----------------------------------------------------------------------------
const ACE_Time_Value& LocalMailbox::getPostDefaultTimeout()
{
   return ACE_Time_Value::zero;
}//end getPostDefaultTimeout


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the Debug flag value for this mailbox
// Design:
//-----------------------------------------------------------------------------
int LocalMailbox::getDebugValue()
{
   return debugValue_;
}//end getDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Sets the Debug flag value for this mailbox
// Design:
//-----------------------------------------------------------------------------
void LocalMailbox::setDebugValue(int debugValue)
{
   debugValue_ = debugValue;
}//end setDebugValue


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the mailbox address
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& LocalMailbox::getMailboxAddress()
{
   return localAddress_;
}//end getMailboxAddress


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

