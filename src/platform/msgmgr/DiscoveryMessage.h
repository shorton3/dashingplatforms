/******************************************************************************
* 
* File name:   DiscoveryMessage.h 
* Subsystem:   Platform Services 
* Description: This class implements a remote/distributed type message that 
*              communicates MailboxLookupService updates across remote nodes.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DISCOVERY_MANAGER_MESSAGE_H_
#define _PLAT_DISCOVERY_MANAGER_MESSAGE_H_

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
 * DiscoveryMessage implements a remote/distributed type message that
 * communicates MailboxLookupService updates across remote nodes. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Supported DiscoveryMessage operations */
typedef enum
{
   DISCOVERY_REGISTER,
   DISCOVERY_DEREGISTER,
   LAST_DISCOVERY_OPERATION
} DiscoveryOperationType;


class DiscoveryMessage : public MessageBase
{
   public:

      /**
       * Constructor
       * @param sourceAddress Mailbox Address for this DiscoveryManager (on this node)
       * @param operation Type of discovery operation (register, deregister, etc.)
       * @param discoveryAddress Mailbox Address that is being registered/deregistered
       */
      DiscoveryMessage(const MailboxAddress& sourceAddress, DiscoveryOperationType operation,
         unsigned int originatingPID, const MailboxAddress& discoveryAddress);

      /**
       * Copy Constructor
       */
      DiscoveryMessage(const DiscoveryMessage& rhs);

      /** Virtual Destructor */
      virtual ~DiscoveryMessage();

      /**
       * Returns the Message Id
       */
      unsigned short getMessageId() const;

      /**
       * Returns the Discovery Operation type
       */
      DiscoveryOperationType getOperationType() const;

      /**
       * Returns the Discovery Address which is being registered/deregistered
       */
      const MailboxAddress& getDiscoveryAddress() const;

      /**
       * Returns the PID of the process that originated this message. Used to verify
       * and filter out messages from our own DiscoveryManager
       */
      unsigned int getOriginatingPID() const;

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
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      DiscoveryMessage& operator= (const DiscoveryMessage& rhs);

      /** Type of discovery operation (register, deregister, etc.) */
      DiscoveryOperationType operation_;

      /** Mailbox Address that is being registered/deregistered */
      MailboxAddress discoveryAddress_;

      /** PID of the process that originated this message */
      unsigned int originatingPID_;
};

#endif
