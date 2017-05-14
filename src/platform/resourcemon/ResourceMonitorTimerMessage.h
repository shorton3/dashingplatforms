/******************************************************************************
* 
* File name:   ResourceMonitorTimerMessage.h 
* Subsystem:   Platform Services 
* Description: Timer Message for periodically scheduling collection of resource
*              usage statistics.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_RESOURCE_MONITOR_TIMER_MESSAGE_H_
#define _PLAT_RESOURCE_MONITOR_TIMER_MESSAGE_H_

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
 * ResourceMonitorTimerMessage is a Timer Message for periodically
 * scheduling collection of resource usage statistics.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ResourceMonitorTimerMessage : public TimerMessage
{
   public:

      /** Constructor */
      ResourceMonitorTimerMessage(const MailboxAddress& sourceAddress, 
                              ACE_Time_Value timeout,
                              ACE_Time_Value restartInterval);

      /** Virtual Destructor */
      virtual ~ResourceMonitorTimerMessage();

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
      ResourceMonitorTimerMessage(const ResourceMonitorTimerMessage& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ResourceMonitorTimerMessage& operator= (const ResourceMonitorTimerMessage& rhs);

};

#endif
