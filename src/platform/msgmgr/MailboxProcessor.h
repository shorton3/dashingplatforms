/******************************************************************************
* 
* File name:   MailboxProcessor.h 
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

#ifndef _PLAT_MAILBOX_PROCESSOR_H_
#define _PLAT_MAILBOX_PROCESSOR_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

// For C++ class declarations, we have one (and only one) of these access 
// blocks per class in this order: public, protected, and then private.
//
// Inside each block, we declare class members in this order:
// 1) nested classes (if applicable)
// 2) static methods
// 3) static data
// 4) instance methods (constructors/destructors first)
// 5) instance data
//

/**
 * MailboxProcessor reads the messages from the Mailbox message queue and calls
 * the appropriate Message Handler. 
 * <p>
 * By default, the mailbox processor remains blocked on the mailbox queue waiting
 * for messages. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageHandlerList;
class MailboxOwnerHandle;

class MailboxProcessor
{
   public:

      /** Constructor. Associate a Mailbox with its Message Handler List */
      MailboxProcessor(MessageHandlerList* handlerList, MailboxOwnerHandle& mailboxOwnerHandle);

      /** 
       * Virtual Destructor.
       */
      virtual ~MailboxProcessor();

      /** 
       * Reads messages from the queue and calls the appropriate message handler.
       * <p> 
       * If the queue is empty it will block.  After the message has been processed 
       * it will be deleted (or it will be checked in to the OPM if it is an OPM 
       * managed object). So applications SHOULD NOT DELETE messages when they 
       * handle them. If the mailbox has not be activated by the application, then
       * getMessage() will return NULL, and this method will return.
       * <p>
       * @param numberThreads if more than 1 thread is given as a parameter, then a 
       *    ThreadManager thread pool will be created to service the Mailbox queue.
       * NOTE!! If the developer chooses to use a ThreadPool for processing the
       *    the Mailbox queue, the developer is responsible for ensuring that all 
       *    of the Mailbox Handlers are THREAD SAFE!!!
       */
      void processMailbox(int numberThreads = 1);

   protected:

   private:

      /** Perform the actual message dequeuing */
      void processMailboxInternal();

      /** Static invocation method for starting processMailboxInternal in multiple threads */
      static void invokeStatic();

      /** Default Constructor.*/
      MailboxProcessor();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MailboxProcessor(const MailboxProcessor& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MailboxProcessor& operator= (const MailboxProcessor& rhs);

      /** Mailbox to process */
      MessageHandlerList* handlerList_;

      /** Message Handler List */
      MailboxOwnerHandle& mailboxOwnerHandle_;

      /**
       * Static instance pointer
       */
      static MailboxProcessor* mailboxProcessor_;
};

#endif
