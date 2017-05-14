/******************************************************************************
* 
* File name:   DiscoveryLocalMessage.h 
* Subsystem:   Platform Services 
* Description: This class implements a local type message that is used for 
*              sending local commands to the DiscoveryManager mailbox (for
*              registering/deregistering and displaying local entries)
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_DISCOVERY_MANAGER_LOCAL_MESSAGE_H_
#define _PLAT_DISCOVERY_MANAGER_LOCAL_MESSAGE_H_

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
 * DiscoveryLocalMessage implements a local type message that is used for sending
 * local commands to the DiscoveryManager mailbox (for registering/deregistering
 * and displaying local entries).
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Supported Discovery Local Message operations */
typedef enum
{
   DISCOVERY_LOCAL_REGISTER,
   DISCOVERY_LOCAL_DEREGISTER,
   DISCOVERY_LOCAL_DISPLAY,
   LAST_LOCAL_DISCOVERY_OPERATION
} DiscoveryLocalOperationType;


class DiscoveryLocalMessage : public MessageBase
{
   public:

      /**
       * Constructor
       * @param sourceAddress Mailbox Address for this DiscoveryManager (on this node)
       * @param operation Type of discovery operation (register, deregister, display, etc.)
       * @param discoveryAddress Mailbox Address that is being registered/deregistered
       */
      DiscoveryLocalMessage(const MailboxAddress& sourceAddress, DiscoveryLocalOperationType operation,
         const MailboxAddress& discoveryAddress);

      /**
       * Copy Constructor
       */
      DiscoveryLocalMessage(const DiscoveryLocalMessage& rhs);

      /** Virtual Destructor */
      virtual ~DiscoveryLocalMessage();

      /**
       * Returns the Message Id
       */
      unsigned short getMessageId() const;

      /**
       * Returns the Discovery Operation type
       */
      DiscoveryLocalOperationType getOperationType() const;

      /**
       * Returns the Discovery Address which is being registered/deregistered
       */
      const MailboxAddress& getDiscoveryAddress() const;

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
      DiscoveryLocalMessage& operator= (const DiscoveryLocalMessage& rhs);

      /** Type of discovery operation (register, deregister, display, etc.) */
      DiscoveryLocalOperationType operation_;

      /** Mailbox Address that is being registered/deregistered */
      MailboxAddress discoveryAddress_;
};

#endif
