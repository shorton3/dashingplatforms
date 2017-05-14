/******************************************************************************
* 
* File name:   ReusableMessageBase.h 
* Subsystem:   Platform Services 
* Description: Inherits from the MessageBase to provide for Messages that are
*              reusable/resendable without the MsgMgr framework consuming them;
*              that is, they will not be consumed when the MsgMgr is done 
*              processing them (so they can be sent over and over again).
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_REUSABLE_MESSAGE_BASE_H_
#define _PLAT_REUSABLE_MESSAGE_BASE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageBase.h"

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
 * ReusableMessageBase inherits from the MessageBase to provide for Messages 
 * that are reusable/resendable without the MsgMgr framework consuming them;
 * that is, they will not be consumed when the MsgMgr is done
 * processing them (so they can be sent over and over again).
 * <p>
 * Once the ReusableMessageBase is no longer needed (in the case the Message
 * is recurring for only a finite number of times), the developer can mark
 * the message as no longer reusable which will cause the MsgMgr framework
 * to delete it when the current/next iteration is complete.
 * <p>
 * ReusableMessageBase also provides the developer with a way to mark the
 * message as 'havingBeenProcessed' in its current iteration, as well as
 * a way to reset that condition on the next iteration.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class ReusableMessageBase : public MessageBase
{
   public:

      /** Constructor */
      ReusableMessageBase(const MailboxAddress& sourceAddress,
                  unsigned int versionNumber,
                  unsigned int sourceContextId = 0,
                  unsigned int destinationContextId = 0);

      /** Virtual Destructor */
      virtual ~ReusableMessageBase();

      /** Mutator method to set whether or not the Message has already been processed this time around */
      void setHasBeenProcessed(bool hasBeenProcessed);

      /** Accessor method used to determine if the Message has already been processed this time around */
      bool hasBeenProcessed();

      /** Mark this Message as no longer reusable so that it can be consumed by the MsgMgr framework */
      void setNoLongerReusable();

   protected:

   private:

      /** Default Constructor */
      ReusableMessageBase();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      ReusableMessageBase(const ReusableMessageBase& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      ReusableMessageBase& operator= (const ReusableMessageBase& rhs);

      /** Flag to indicate whether or not the ReusableMessage has already been processed */
      bool hasBeenProcessed_;

};

#endif
