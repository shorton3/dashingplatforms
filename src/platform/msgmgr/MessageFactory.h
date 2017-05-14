/******************************************************************************
* 
* File name:   MessageFactory.h 
* Subsystem:   Platform Services 
* Description: This utility class performs the re-creation of messages from
*              the network socket stream back into their object form. The
*              applications are responsible for registering messages with the
*              MessageFactory upon initialization--this is how the MessageFactory
*              knows about the various message types.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_FACTORY_H_
#define _PLAT_MESSAGE_FACTORY_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Thread_Mutex.h>
#include <map>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "Callback.h"

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
 * MessageFactory is a utility class that performs re-creation of
 * received messages from the network socket stream back into their object
 * form.
 * <p>
 * Applications are responsible for registering messages with the Message
 * Factory upon initialization--this is how the MessageFactory knows about
 * the various message types.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class MessageBase;
class MessageBuffer;

typedef CBFunctor1wRet<MessageBuffer*, MessageBase*> MessageBootStrapMethod;

class MessageFactory
{
   public:

      /** Virtual Destructor */
      virtual ~MessageFactory();

      /**
       * Allows applications to register Message Factory support for a
       * specified Message Id and supplied static initializer (bootstrap) method.
       * <p>
       * Developers should note that the static bootstrap method is most usually the
       * overriden static 'deserialize' method implemented within the Message class
       * (inherited from MessageBase). However, if complex deserialization logic is
       * required (or if deserialization is dependent on application variables), any
       * static bootstrap method can be supplied here. 
       */
      static void registerSupport(unsigned short messageId, MessageBootStrapMethod& messageCreator);

      /**
       * Convert the buffer back into its Message object form
       */
      static MessageBase* recreateMessageFromBuffer(MessageBuffer& buffer);

      /**
       * Output registered message Ids
       */
      static void listRegisteredMessages();

      /** 
       * Map to implement the registry for mapping Message Bootstrap methods to their
       * Message Ids 
       */
      typedef map<unsigned short, MessageBootStrapMethod> MessageFactoryRegistry;

   protected:

   private:

      /** Registry Map */
      static MessageFactoryRegistry messageFactoryRegistry_;

      /** Non-recursive Mutex for protecting Map (registry) access */
      static ACE_Thread_Mutex messageFactoryMutex_;

      /** Constructor */
      MessageFactory();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      MessageFactory(const MessageFactory& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      MessageFactory& operator= (const MessageFactory& rhs);

};

#endif
