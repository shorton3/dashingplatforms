/******************************************************************************
* 
* File name:   MessageTestTimerMessage.h 
* Subsystem:   Platform Services 
* Description: Test Message #1
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_TEST_TIMER_MESSAGE_H_
#define _PLAT_MESSAGE_TEST_TIMER_MESSAGE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/msgmgr/TimerMessage.h"

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
 * MessageTestTimerMessage is Test Message #1. 
 * <p>
 * This message demonstrates timer message passing only.
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageTestTimerMessage : public TimerMessage
{
   public:

      /** Constructor */
      MessageTestTimerMessage(const MailboxAddress& sourceAddress, 
                              ACE_Time_Value timeout,
                              ACE_Time_Value restartInterval);

      /** Constructor */
      MessageTestTimerMessage(const MailboxAddress& sourceAddress,
                              ACE_Time_Value timeout);

      /** Virtual Destructor */
      virtual ~MessageTestTimerMessage();

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
      MessageTestTimerMessage(const MessageTestTimerMessage& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageTestTimerMessage& operator= (const MessageTestTimerMessage& rhs);

};

#endif
