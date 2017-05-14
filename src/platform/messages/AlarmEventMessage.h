/******************************************************************************
* 
* File name:   AlarmEventMessage.h 
* Subsystem:   Platform Services 
* Description: AlarmEventMessage is used for delivery of Alarm and Event Report
*              notifications to the EMS' Distributed Mailbox.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_ALARM_EVENT_MESSAGE_H_
#define _PLAT_MESSAGE_ALARM_EVENT_MESSAGE_H_

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
 * AlarmEventMessage is used for delivery of Alarm and Event Report notifications
 * to the EMS' Distributed Mailbox. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Enum for defining message type as alarm, clear alarm, or event report */
typedef enum
{
   ALARM_MESSAGE,
   CLEAR_ALARM_MESSAGE,
   EVENT_REPORT_MESSAGE
} AlarmEventMessageType;


class AlarmEventMessage : public MessageBase
{
   public:

      /** Constructor */
      AlarmEventMessage(const MailboxAddress& sourceAddress, AlarmEventMessageType messageType,
         string neid, unsigned short managedObject, unsigned int managedObjectInstance,
         unsigned short alarmCode, unsigned short eventCode, unsigned int pid, unsigned int timeStamp,
         unsigned int sourceContextId = 0, unsigned int destinationContextId = 0);

      /** Virtual Destructor */
      virtual ~AlarmEventMessage();

      /**
       * Returns the Message Type (Alarm, Clear, EventReport)
       */
      AlarmEventMessageType getMessageType() const;

      /**
       * Returns the Message Id
       */
      unsigned short getMessageId() const;

      /**
       * Returns the NEID that the alarm applies to
       */
      string getNEID() const;

      /**
       * Returns the managed object class
       */
      unsigned short getManagedObject() const;

      /**
       * Returns the managed object instance
       */
      unsigned int getManagedObjectInstance() const;

      /**
       * Returns the alarm code
       */
      unsigned short getAlarmCode() const;

      /**
       * Returns the event code
       */
      unsigned short getEventCode() const;

      /**
       * Returns the process id
       */
      unsigned int getPid() const;

      /**
       * Returns the timestamp
       */
      unsigned int getTimestamp() const;

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
      AlarmEventMessage(const AlarmEventMessage& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      AlarmEventMessage& operator= (const AlarmEventMessage& rhs);

      /** Message Type (Alarm, Clear, Event Report) */
      AlarmEventMessageType messageType_;

      /** NEID that the Alarm applies to */
      string neid_;

      /** Managed Object class */
      unsigned short managedObject_;

      /** Managed Object Instance */
      unsigned int managedObjectInstance_;

      /** Alarm Code. Only either Alarm Code or Event Code is used. */
      unsigned short alarmCode_;

      /** Event Code. Only either Alarm Code or Event Code is used. */
      unsigned short eventCode_;

      /** Process Id */
      unsigned int pid_;

      /** Timestamp of when the alarm was generated */
      unsigned int timeStamp_;

};

#endif
