/******************************************************************************
* 
* File name:   TimerMessage.h 
* Subsystem:   Platform Services 
* Description: Base class implementation for Timer Messages. All system
*              software Timer Messages (that can tolerate the MsgMgr concurrency
*              latency) must inherit from this class.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_TIMER_MESSAGE_H_
#define _PLAT_TIMER_MESSAGE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Time_Value.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "ReusableMessageBase.h"
#include "MailboxAddress.h"

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
 * TimerMessage is the base class implementation for all system timer messages. 
 * <p>
 * All system software Timer Messages (that can tolerate the MsgMgr concurrency
 * latency) must inherit from this class. The exception to this rule is that
 * this class can be created concrete--If an application has no need for passing
 * parameters/values along with the Timer, then it can use this generic/concrete
 * class to get an expiration time.
 * <p>
 * High resolution or hardware timers (sub-microsecond) must register or schedule
 * themselves manually.
 * <p>
 * Because the TimerMessage has the capability to be automatically recurring (that
 * is, it can automatically re-schedule itself), the application developer needs
 * to take special notice. This class does not automatically inherit from OPMBase.
 * If a TimerMessage subclass is required to be OPM pooled, then it will need to
 * do multiple inheritance from both MessageBase (or ReusableMessageBase) and OPMBase.
 * <p>
 * 1. If the TimerMessages are pooled by the OPM, then no special handling is 
 *    required as the MsgMgr framework will handle the release of the timers back
 *    into their respective OPM pools after their handler has completed.
 * 2. Otherwise, the TimerMessage will behave like a ReusableMessageBase if a
 *    non-default 'RestartInterval' parameter is given to make the Timer periodically
 *    recurring. In this case, the application has the responsibility to delete the
 *    TimerMessage when it is no longer needed. Otherwise, if RestartInterval is defaulted,
 *    then the TimerMessage will behave like a MessageBase and be deleted by the MsgMgr.
 * Note: A TimerMessage can be changed to be 'no longer reusable', after which it
 *    will behave like a regular MessageBase after the expiration handler is
 *    complete (or if the Timer is canceled).
 * <p>
 * Also note that if a restartInterval is specified to periodically restart the
 * Timer, then it is NOT necessary to re-post the Timer Message. All other reusable
 * messages must be posted again in order to reuse them.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class TimerMessage : public ReusableMessageBase
{
   public:

      /**
       * Constructor 
       * @param timeout time interval after which the timer will expire 
       * @param restartInterval time interval after which the timer will 
       *    be automatically rescheduled. If this parameter is defaulted,
       *    then the TimerMessage will behave like a MessageBase; otherwise,
       *    if it is non-default, then it will behave like a ReusableMessageBase.
       *    If a non-default restartInterval is specified, it is NOT necessary
       *    to re-post (by calling scheduleTimer() again) the message to
       *    restart the Timer.
       */
      TimerMessage(const MailboxAddress& sourceAddress,
                  unsigned int versionNumber,
                  ACE_Time_Value timeout,
                  ACE_Time_Value restartInterval = ACE_Time_Value::zero,
                  unsigned int sourceContextId = 0,
                  unsigned int destinationContextId = 0);

      /** Virtual Destructor */
      virtual ~TimerMessage();

      /**
       * Return the time interval after which the timer will expire
       */
      ACE_Time_Value& getTimeout();

      /**
       * Return the time interval after which the timer will be automatically rescheduled 
       */
      ACE_Time_Value& getRestartInterval();

      /**
       * Sets the time that the Timer actually expires. This is called
       * by the MsgMgr framework. Applications should NOT call this method
       */
      void setExpirationTime(ACE_Time_Value expirationTime);

      /**
       * Retrieves the 'actual' time that the Timer Expired. This value is set
       * by the MsgMgr framework, and it is provided to the applications so that
       * it can be used to 'judge' the amount of latency introduced by the Timer
       * sitting inside the mailbox queue.
       */
      ACE_Time_Value& getExpirationTime();

      /**
       * Returns the unique message identifier for any subclass of MessageBase.
       * MessageId's must be registered in common/MessageIds.h
       */
      virtual unsigned short getMessageId() const;

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /** Time interval after which the timer will expire */
      ACE_Time_Value timeout_;

      /** Time interval after which the timer will be automatically rescheduled */
      ACE_Time_Value restartInterval_;

      /** Records the 'actual' time that the Timer expired */
      ACE_Time_Value expirationTime_;

   private:

      /** Default Constructor */
      TimerMessage();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      TimerMessage(const TimerMessage& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      TimerMessage& operator= (const TimerMessage& rhs);

};

#endif
