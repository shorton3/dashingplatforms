/******************************************************************************
* 
* File name:   MailboxHandle.h 
* Subsystem:   Platform Services 
* Description: This class is a handle to a mailbox. The mailbox itself may be
*              of local or distributed type. The mailbox handles prevent 
*              application developers from getting direct references
*              to the mailboxes themselves.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MAILBOX_HANDLE_H_
#define _PLAT_MAILBOX_HANDLE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxOwnerHandle.h"

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
 * MailboxHandle is a developer handle to a local or remote type mailbox. 
 * <p>
 * The mailbox handles prevent application developers from getting
 * direct references to the mailboxes themselves. This is the programmatic
 * interface that developers use to access the mailbox functionality.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MailboxHandle 
{
   public:

      /** Constructor. Stores a pointer to the mailbox */
      MailboxHandle(MailboxBase* mailboxPtr);

      /** Virtual Destructor */
      virtual ~MailboxHandle();

      /**
       * Post a message to this mailbox
       * @returns ERROR for an error; otherwise OK
       */
      virtual int post(MessageBase* messagePtr, const ACE_Time_Value* timeout = &ACE_Time_Value::zero);

      /** Returns the Debug flag value for this mailbox. */
      virtual int getDebugValue();

      /** Sets the Debug flag value for this mailbox. */
      virtual void setDebugValue(int debugValue);

      /** Return whether this mailbox is a remote proxy or not */
      virtual bool isProxy();

      /** Return the mailbox address */
      virtual MailboxAddress& getMailboxAddress();

      /** Return the sent message counter value */
      virtual unsigned int getSentCount();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

      /** Pointer to the encapsulated mailbox */
      MailboxBase* mailboxPtr_;

   private:

      /** Default Constructor */
      MailboxHandle();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MailboxHandle(const MailboxHandle& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MailboxHandle& operator= (const MailboxHandle& rhs);

};

#endif
