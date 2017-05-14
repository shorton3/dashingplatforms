/******************************************************************************
* 
* File name:   MessageTestGroupMessage.h 
* Subsystem:   Platform Services 
* Description: Test Message for Group Mailbox
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_TEST_GROUP_MESSAGE_H_
#define _PLAT_MESSAGE_TEST_GROUP_MESSAGE_H_

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
 * MessageTestGroupMessage is a Test Message for the Group Mailbox. 
 * <p>
 * This message demonstrates distributed and local mailbox message passing.
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageTestGroupMessage : public MessageBase
{
   public:

      /** Constructor */
      MessageTestGroupMessage(const MailboxAddress& sourceAddress, int ourIntValue, string ourStringValue);

      /** Virtual Destructor */
      virtual ~MessageTestGroupMessage();

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
      MessageTestGroupMessage(const MessageTestGroupMessage& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageTestGroupMessage& operator= (const MessageTestGroupMessage& rhs);

      /** Test int value */
      int ourIntValue_;

      /** Test string value */
      string ourStringValue_;

};

#endif
