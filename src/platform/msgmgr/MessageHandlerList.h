/******************************************************************************
* 
* File name:   MessageHandlerList.h 
* Subsystem:   Platform Services 
* Description: The MessageHandlerList class maintains a mapping between each 
*              specific Message (based on MessageId) and its corresponding
*              Message Handler method.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_HANDLER_LIST_H_
#define _PLAT_MESSAGE_HANDLER_LIST_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <map>

#include <ace/Thread_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "Callback.h"

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------

class MessageBase;

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
 * MessageHandlerList class maintains a mapping between each specific Message
 * (based on MessageId) and its corresponding Message Handler method.
 * <p>
 * MessageHandlerList uses Functors to turn c-style functions into objects.
 * These function objects (or Functors) can then be registered using a key
 * (the MessageId) to associate the handler function to the receipt of a 
 * particular message.
 * <p>
 * MessageHandlerList does not implement any type of thread safety mechanisms.
 * This means that the application developer must build the handler list
 * from a SINGLE THREAD. DO NOT ATTEMPT to add message handlers from multiple
 * threads!
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/**
 * Currently only use function pointers that accept a single MessageBase
 * parameter and return an int
 */                                                                                                         
typedef CBFunctor1wRet<MessageBase *, int> MessageHandler;

/** Map is used to maintain the message handler registry */
typedef map<unsigned short, MessageHandler, less<unsigned short> > MessageHandlerRegistryType;

class MessageHandlerList
{
   public:

      /** Constructor */
      MessageHandlerList();

      /** Virtual Destructor */
      virtual ~MessageHandlerList();

      /** 
       * Perform the mapping between message Id and handler function 
       * @returns true if the mapping was successful
       */
      bool add(unsigned short messageId, const MessageHandler& handler);

      /** Remove the mapping between message Id and handler function */
      void remove(unsigned short messageId);

      /**
       * Return a handler for a given message Id.
       * @returns default handler if message Id could not be found
       */
      const MessageHandler& find(unsigned short messageId);

      /** Change the default handler */
      void setDefault(MessageHandler& defaultHandler);

      /** Restore the default message handler */
      void restoreDefault();

      /** List all mappings between message Id and handler functions */
      void listAllMessageHandlers();

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
      MessageHandlerList(const MessageHandlerList& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageHandlerList& operator= (const MessageHandlerList& rhs);

      /** Default message handler */
      int defaultMessageHandler(MessageBase* messagePtr);

      /** The functor object that contains the default message handler function */
      MessageHandler defaultHandler_;

      /**
       * The functor object that contains the default message handler that is
       * currently active.
       */
      MessageHandler currentDefaultHandler_;

      /** Map is used to maintain the message handler registry list. */
      MessageHandlerRegistryType messageHandlerList_;

      /** Non-recursive Ace Thread Mutex that protects/guards this list */
      ACE_Thread_Mutex listMutex_;
};

#endif
