/******************************************************************************
*
* File name:   MessageFactory.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sstream>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageFactory.h"
#include "MessageBase.h"
#include "MessageBuffer.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Map to implement the registry for mapping Message Bootstrap methods to their
// Message Ids
MessageFactory::MessageFactoryRegistry MessageFactory::messageFactoryRegistry_;

// Mutex for protecting Map (registry) access
ACE_Thread_Mutex MessageFactory::messageFactoryMutex_;

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageFactory::MessageFactory()
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageFactory::~MessageFactory()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: MessageBase subclasses that need to be de-serialized (re-created)
//              must register themselves with the MessageFactory so that it knows
//              how to perform the recreation.
// Design:
//-----------------------------------------------------------------------------
void MessageFactory::registerSupport(unsigned short messageId, MessageBootStrapMethod& messageCreator)
{
   TRACELOG(DEBUGLOG, MSGMGRLOG, "Message Factory Registering: message Id 0x%x", messageId,0,0,0,0,0);
                                                                                                           
   // Map protection
   messageFactoryMutex_.acquire();
                                                                                                           
   // First check to see if this messageId already has been registered, if so, 
   // we want to delete the registration and replace it
   MessageFactoryRegistry::iterator registryIterator = messageFactoryRegistry_.find(messageId);
   if (registryIterator != messageFactoryRegistry_.end())
   {
      messageFactoryRegistry_.erase(registryIterator);
   }//end if

   // Perform the registration
   pair<MessageFactoryRegistry::iterator, bool> insertResult;
   insertResult = messageFactoryRegistry_.insert(make_pair(messageId, messageCreator));
   // Check to see if the map insert was successful
   if (!insertResult.second)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "MessageFactoryRegistry insertion failed",0,0,0,0,0,0);
   }//end if

   // Map protection
   messageFactoryMutex_.release();
}//end registerSupport


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Convert the buffer back into its Message object form
// Design:
//-----------------------------------------------------------------------------
MessageBase* MessageFactory::recreateMessageFromBuffer(MessageBuffer& buffer)
{
   unsigned short messageId = 0;

   buffer >> messageId;

   MessageFactoryRegistry::iterator registryIterator = messageFactoryRegistry_.find(messageId);
   if ( registryIterator == messageFactoryRegistry_.end() )
   {
      ostringstream ostr;
      ostr << "No bootstrap method registered for message Id 0x" << hex << messageId << ends;
      STRACELOG(WARNINGLOG, MSGMGRLOG, ostr.str().c_str()); 
      return NULL;
   }//end if
   else
   {
      return (registryIterator->second(&buffer));
   }//end else
}//recreateMessageFromBuffer


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Output registered message Ids
// Design:
//-----------------------------------------------------------------------------
void MessageFactory::listRegisteredMessages()
{
   ostringstream ostr;

   ostr << "Message Factory Registered messages: ";
 
   MessageFactoryRegistry::iterator registryIterator;
   for ( registryIterator = messageFactoryRegistry_.begin(); registryIterator != 
         messageFactoryRegistry_.end(); registryIterator++)
   {
      ostr << "0x" << hex << registryIterator->first << ", ";
   }//end for
   ostr << ends;

   STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());
}//end listRegisteredMessages


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

