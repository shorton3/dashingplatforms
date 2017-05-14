/******************************************************************************
* 
* File name:   MessageBase.h 
* Subsystem:   Platform Services 
* Description: This is the base class for all messages. This is an abstract
*              class and cannot be instantiated. All messages in the system
*              will be derived from this base class.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_BASE_H_
#define _PLAT_MESSAGE_BASE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxAddress.h"

#include "platform/opm/ObjectBase.h"

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

class MessageBuffer;

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
 * MessageBase is the base class for all messages. 
 * <p>
 * This is an abstract class and cannot be instantiated. All messages in 
 * the system will be derived ultimately from this base class.
 * <p>
 * The following 4 types of message subclasses are possible in the MsgMgr:
 *  - Non-reusable / Not OPM Pooled
 *        Example: Distributed Message that will only be used once at initialization
 *  - Non-reusable / OPM Pooled
 *        Example: Messages used for different purposes within Callp
 *  - Reusable / Not OPM Pooled
 *        Example: Periodically recurring TimerMessage, but only one is needed
 *        for the application's lifetime.
 *  - Reusable / OPM Pooled
 *        Example: Many TimerMessages are needed by the application. Each Timer
 *        will be periodically recurring for a finite number of times before
 *        being released back into the OPM.
 * <p>
 * Message subclasses do not automatically inherit from OPMBase. If a message
 * subclass is required to be OPM poolable, then it will need to do multiple
 * inheritance from both MessageBase (or ReusableMessageBase) and OPMBase. This
 * is ALSO true for timers which inherit from the TimerMessage base class. Note
 * that TimerMessage will behave like either a MessageBase or ReusableMessageBase
 * depending on whether a non-default 'RestartInterval' parameter is given to 
 * make the Timer periodically recurring.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageBase : public ObjectBase
{
   public:

      /** Constructor */
      MessageBase(const MailboxAddress& sourceAddress,
                  unsigned int versionNumber,
                  unsigned int sourceContextId = 0,
                  unsigned int destinationContextId = 0);

      /** Virtual Destructor */
      virtual ~MessageBase();

      /**
       * Returns static message identifier for any subclass of MessageBase.
       */
      static unsigned short getStaticMessageId();

      /** 
       * Returns the unique message identifier for any subclass of MessageBase.
       * MessageId's must be registered in common/MessageIds.h
       */
      virtual unsigned short getMessageId() const = 0;

      /** Returns the destination mailbox for the message */
      virtual unsigned char getDestinationMailboxId() const;

      /** 
       * Default serialization implementation.
       *
       * This method only needs to be implemented for distributed messages. 
       * @returns OK or ERROR if not successful
       */
      virtual int serialize(MessageBuffer& buffer);

      /**
       * Default static deserialization (bootstrap) implementation.
       *
       * This method only needs to be implemented for distributed messages. Since this
       * method needs to be static in order to register it with the MessageFactory,
       * it does not make sense for it to be virtual. This method is provided here for
       * developer clarity only.
       * @returns Either a 'new' created Messate object with the contents of the buffer,
       *        or an pooled object 'reserved' from its OPM pool and initialized with the
       *        contents of the buffer.
       */
      static MessageBase* deserialize(MessageBuffer* buffer);

      /** Return the application specific context info for identifying the source */
      virtual unsigned int getSourceContextId() const;

      /** Return the application specific context info for identifying the destination */
      virtual unsigned int getDestinationContextId() const;

      /** Return the source mailbox address from which the message was sent */
      virtual const MailboxAddress& getSourceAddress() const;

      /** 
       * Delete the message and perform cleanup operations 
       * 
       * This method is called automatically by the Mailbox Processor. Here we
       * use the ObjectBase class in OPM to determine if the object is managed 
       * by the OPM. If so, then we check the object in; otherwise, we just delete it.
       * <p>
       * Here, a special case exists for the single recurring TimerMessage: Its not
       * pooled/owned by OPM, but it is poolable (inherited from OPMBase, since the
       * TimerMessage base class is). Since its set to periodically restart, we don't
       * want to delete it. This also means that non-recurring TimerMessages WILL NOT
       * BE DELETED ALSO, so its upto the application developer to keep a reference
       * to them and delete them when they are done with them.
       */
      virtual void deleteMessage();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      virtual string toString() = 0;

      /**
       * Flag this message as high priority. This flag will cause the message to be
       * re-ordered in the Mailbox message queue (so will NOT follow FIFO rules).
       * Default priority is 0. Priorities greater than 0 have greater priority.
       */
      void setPriority(unsigned int priorityLevel = 0);

      /**
       * Set the version for this message. This method is used by the MsgMgr framework
       * to re-apply the version number following deserialization
       */
      void setVersion(unsigned int versionNumber);

      /**
       * Return the priority Level for this message.
       * Default priority is 0. Priorities greater than 0 have greater priority.
       */
      unsigned int getPriority();

      /**
       * Get the version number of this message.
       */
      unsigned int getVersion();

      /**
       * Return whether or not this object is managed by an OPM pool
       */
      bool isPoolable();

      /** 
       * Return whether or not this object is Reusable
       */
      bool isReusable();

   protected:

      /**
       * Source mailbox identifier of the application that sent the message
       */
      MailboxAddress sourceAddress_;

      /** 
       * Application specific context for identifying the source. Usage
       * of this value is upto the application.
       */
      unsigned int sourceContextId_;

      /**
       * Application specific context for identifying the destination. Usage
       * of this value is upto the application.
       */ 
      unsigned int destinationContextId_;

      /** Version Number of the message */
      unsigned int versionNumber_;

      /** Whether or not this Message is re-usable. If true, then the MsgMgr framework
          will not consume it. */
      bool isReusable_;

   private:

      /** Default Constructor */
      MessageBase();

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageBase& operator= (const MessageBase& rhs);

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MessageBase(const MessageBase& rhs);

      /** Priority Level assigned to the message */
      unsigned int priorityLevel_;

};

#endif
