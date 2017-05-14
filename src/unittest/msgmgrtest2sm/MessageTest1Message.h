/******************************************************************************
* 
* File name:   MessageTest1Message.h 
* Subsystem:   Platform Services 
* Description: Test Message #1
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_TEST1_MESSAGE_H_
#define _PLAT_MESSAGE_TEST1_MESSAGE_H_

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
 * MessageTest1Message is Test Message #1. 
 * <p>
 * This message demonstrates local mailbox message passing only.
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageTest1Message : public MessageBase
{
   public:

      /** Constructor */
      MessageTest1Message(const MailboxAddress& sourceAddress);

      /** Virtual Destructor */
      virtual ~MessageTest1Message();

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
      MessageTest1Message(const MessageTest1Message& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageTest1Message& operator= (const MessageTest1Message& rhs);

};

#endif
