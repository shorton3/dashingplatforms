/******************************************************************************
* 
* File name:   MessageTestGroup.h 
* Subsystem:   Platform Services 
* Description: Unit test code for the MsgMgr framework. Group Mailbox tests.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_TEST_GROUP_H_
#define _PLAT_MESSAGE_TEST_GROUP_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/msgmgr/MailboxLookupService.h"
#include "platform/msgmgr/MailboxOwnerHandle.h"
#include "platform/msgmgr/MailboxHandle.h"
#include "platform/msgmgr/MessageHandlerList.h"

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
 * MessageTestGroup is a collection of unit test procedures for the MsgMgr framework. 
 * This class deals exclusively with Group Mailboxes.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageTestGroup
{
   public:

      /** Constructor */
      MessageTestGroup();

      /** Virtual Destructor */
      virtual ~MessageTestGroup();

      /** Start the mailbox processing loop */
      void processMailbox();

      /** Returns static singleton instance */
      static MessageTestGroup* getInstance();

      /** Returns pointer to the owner handle of the main mailbox */
      MailboxOwnerHandle* getOwnerHandle();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Called in the constructor to setup the mailbox, activate and
       * to setup handlers.
       * @param groupAddress Mailbox Address of the MessageTestGroup GroupMailbox
       * @returns true if successful
       */
      bool setupMailbox(const MailboxAddress& groupAddress);

      /**
       * Message Handler for Test Group Messages
       * @param message base class object passed by the Functor
       * @returns OK if successful; otherwise ERROR
       */
      int processTestGroupMessage(MessageBase* message);

      /**
       * Message Handler for Test 1 Messages
       * @param message base class object passed by the Functor
       * @returns OK if successful; otherwise ERROR
       */
      int processTest1Message(MessageBase* message);

      /**
       * Message Handler for Timer Messages. Here these are transferred via the
       * local mailbox only.
       */
      int processTimerMessage(MessageBase* message);

      /** Static singleton instance */
      static MessageTestGroup* messageTestInstance_;

      /** MessageTestGroup Mailbox */
      MailboxOwnerHandle* messageTestGroupMailbox_;

      /** Message Handler List for storing Functors for Mailbox handlers */
      MessageHandlerList* messageHandlerList_;

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MessageTestGroup(const MessageTestGroup& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageTestGroup& operator= (const MessageTestGroup& rhs);

};

#endif
