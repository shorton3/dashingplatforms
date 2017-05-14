/******************************************************************************
* 
* File name:   MessageTest2Message.h 
* Subsystem:   Platform Services 
* Description: Test Message #2
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_TEST2_MESSAGE_H_
#define _PLAT_MESSAGE_TEST2_MESSAGE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

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
 * MessageTest2Message is Test Message #2. 
 * <p>
 * This message demonstrates local mailbox message passing only.
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageTest2Message : public MessageBase
{
   public:

      /** Constructor */
      MessageTest2Message(const MailboxAddress& sourceAddress);

      /** Virtual Destructor */
      virtual ~MessageTest2Message();

      /**
       * Returns the Message Id
       */
      unsigned short getMessageId() const;

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
      MessageTest2Message(const MessageTest2Message& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageTest2Message& operator= (const MessageTest2Message& rhs);

};

#endif
