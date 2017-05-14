/******************************************************************************
* 
* File name:   MessageBlockWrapper.h 
* Subsystem:   Platform Services 
* Description: This class creates a wrapper around the ACE Message Block
*              class so that we can pool ACE Message Blocks inside the OPM. 
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_BLOCK_WRAPPER_H_
#define _PLAT_MESSAGE_BLOCK_WRAPPER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Message_Block.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/opm/OPMBase.h"

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
 * MessageBlockWrapper is a wrapper class around the ACE_Message_Block class. 
 * <p>
 * This simple wrapper exists so that we can create a pool of 
 * ACE_Message_Blocks inside the OPM.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageBlockWrapper : public OPMBase, ACE_Message_Block
{
   public:

      /** Constructor */
      MessageBlockWrapper(int blockSize = 0);

      /** Virtual Destructor */
      virtual ~MessageBlockWrapper();

      /** OPMBase static initializer method for bootstrapping the objects */
      static OPMBase* initialize(int initializer);

      /** OPMBase clean method gets called when the object gets released back
          into its pool */
      void clean();

      /** Overridden ACE_Message_Block Method */
      void base(char *data, size_t size);

      /** Overridden ACE_Message_Block Method */
      char *base (void) const;

      /** Overridden ACE_Message_Block Method */
      void msg_priority (unsigned long priority);

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
      MessageBlockWrapper(const MessageBlockWrapper& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageBlockWrapper& operator= (const MessageBlockWrapper& rhs);

};

#endif
