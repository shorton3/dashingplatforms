/******************************************************************************
* 
* File name:   MailboxAddress.h 
* Subsystem:   Platform Services 
* Description: MailboxAddress encapsulates all of the necessary information
*              for identifying the various types of Mailboxes in the system.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MAILBOX_ADDRESS_H_
#define _PLAT_MAILBOX_ADDRESS_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>
#include <sstream>

#include <ace/INET_Addr.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/common/Defines.h"

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


/** Mailbox Location describes the type of communication the Mailbox is capable of */
enum MailboxLocationType
{
   UNKNOWN_MAILBOX_LOCATION = 0,
   /** For threads within the same process to exchange messages via pointer.
       No Message Copy Made. */
   LOCAL_MAILBOX, 
   /** For processes on the same node/machine to exchange messages via shared
       memory queue. Message Copy is Made, but No Serialization/Deserialization performed */
   LOCAL_SHARED_MEMORY_MAILBOX,
   /** For processes on different nodes to exchange messages via TCP/IP socket.
       Message Serialization/Deserialization is performed */
   DISTRIBUTED_MAILBOX,
   /** For processes to post messages to multiple different nodes simultaneously 
       via Reliable Multicast (Datagram) Protocol. Message Serialization/Deserialization is performed */
   GROUP_MAILBOX,
};

/** Mailbox Address Type describes whether the Mailbox plays is associated
    with a Physical function (such as Card Manager performs a function on each
    physical card) or a Logical function (such as a Routing Manager which performs
    logical function which may be shared between the two mates of a redundant pair.
*/ 
enum MailboxAddressType
{
   UNKNOWN_MAILBOX_TYPE = 0,
   PHYSICAL_MAILBOX,
   LOGICAL_MAILBOX,
};

/**
 * MailboxAddress encapsulates all of the necessary information for
 * identifying the various types of Mailboxes in the system. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

struct MailboxAddress
{
   public:

      /** Defines this mailbox as playing a physical or logical role. Physical roles are
          typically associated with each specific card. Logical roles may be shared 
          across physical card barriers, such as with Redundancy */ 
      MailboxAddressType mailboxType;

      /** Defines the type of communication this mailbox will be involved with (is capable of):
          Local, Local Shared Memory, Distributed, Group */
      MailboxLocationType locationType;
 
      /** Geographic shelf number that the Mailbox resides in */
      int shelfNumber;

      /** Geographic slot number that the Mailbox resides in */
      int slotNumber;

      /** Application-given well-known mailbox name. */
      string mailboxName;

      /** NEID (Network Element Identifier) of the node/card */
      string neid;
 
      /** IP Address and Port Number for the Mailbox. Loopback IP (127.0.0.1) can be used
          for higher performance with applications that will only communicate on-card */
      ACE_INET_Addr inetAddress;

      /** Redundancy Role for the Application Mailbox: Active, Standby, Loadshared */
      PreferredRedundantRole redundantRole;

      /** Constructor */
      MailboxAddress();

      /** Virtual Destructor */
      virtual ~MailboxAddress();

      /** Reset all member variables */
      void reset();

      /** Copy Constructor */
      MailboxAddress(const MailboxAddress& rhs);

      /** Overloaded Assignment Operator */
      MailboxAddress& operator= ( const MailboxAddress &rhs );

      /** Overloaded Equality Operator */
      bool operator== ( const MailboxAddress &rhs ) const;

      /** Overloaded Comparison Operator. NOTE: Currently only base comparison on name, location, IP */
      bool operator< (const MailboxAddress &rhs) const;

      /** Overloaded Comparison Operator. Not implemented!! */
      bool operator<= (const MailboxAddress &rhs) const;

      /** Overloaded Comparison Operator. Not implemented!! */
      bool operator!= (const MailboxAddress &rhs) const;

      /** Overloaded Comparison Operator. Not implemented!! */
      bool operator> (const MailboxAddress &rhs) const;

      /** Overloaded Comparison Operator. Not implemented!! */
      bool operator>= (const MailboxAddress &rhs) const;

      /**
       * Performs a filter matching algorithm based on non-defaulted fields in the
       * Mailbox Address structure. At least 1 field must be non-default and must
       * match for this to return true; also, for LocalMailbox LocationTypes, we
       * only check the LocationType and the MailboxName.
       * @returns true if the addressToCheck is a match to all non-default fields
       * in the matchCriteria.
       */
      static bool isMatchingAddress(const MailboxAddress& matchCriteria, const MailboxAddress& addressToCheck);

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString() const;

      /** Static default ACE_INET_Addr with unknown IP and port (for defaults) */
      static const ACE_INET_Addr defaultUnknownInetAddress_;

   protected:

   private:

};

#endif
