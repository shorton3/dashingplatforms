/******************************************************************************
* 
* File name:   MessageTestRemoteMessage.h 
* Subsystem:   Platform Services 
* Description: Test Message for Distributed Mailbox
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_TEST_REMOTE_MESSAGE_H_
#define _PLAT_MESSAGE_TEST_REMOTE_MESSAGE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/msgmgr/MessageBase.h"

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
 * MessageTestRemoteMessage is a Test Message for the Distributed Mailbox. 
 * <p>
 * This message demonstrates distributed and local mailbox message passing.
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageTestRemoteMessage : public MessageBase
{
   public:

      /** Constructor */
      MessageTestRemoteMessage(const MailboxAddress& sourceAddress, int ourIntValue, string ourStringValue);

      /** Virtual Destructor */
      virtual ~MessageTestRemoteMessage();

      /**
       * Returns the Message Id
       */
      unsigned short getMessageId() const;

      /**
       * Subclassed serialization implementation
       */
      int serialize(MessageBuffer& buffer);

      /**
       * Subclassed deserialization / bootstrap implementation
       */
      static MessageBase* deserialize(MessageBuffer* buffer);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MessageTestRemoteMessage(const MessageTestRemoteMessage& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageTestRemoteMessage& operator= (const MessageTestRemoteMessage& rhs);

      /** Test int value */
      int ourIntValue_;

      /** Test string value */
      string ourStringValue_;

};

#endif
