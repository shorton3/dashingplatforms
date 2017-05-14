/******************************************************************************
*
* File name:   MessageHandlerList.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sstream>

#include <ace/Guard_T.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageHandlerList.h"
#include "MessageBase.h"

#include "platform/logger/Logger.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: The constructor creates a default message handler that will
//              be used when a message does not have a handler associated with it.
// Design:      It uses the makeFunctor() function provided by the 3rd party
//              library we are using ( Callback.cpp and Callback.h ) 
//-----------------------------------------------------------------------------
MessageHandlerList::MessageHandlerList()
   : defaultHandler_ ( makeFunctor( (MessageHandler *) 0, *this, &MessageHandlerList::defaultMessageHandler)),
     currentDefaultHandler_(defaultHandler_)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageHandlerList::~MessageHandlerList()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Perform the mapping between message Id and handler function
// Design:
//-----------------------------------------------------------------------------
bool MessageHandlerList::add(unsigned short messageId, const MessageHandler& handler)
{
   ACE_GUARD_RETURN (ACE_Thread_Mutex, ace_mon, listMutex_, false);

   typedef pair<unsigned short, MessageHandler> MessageHandlerPair;
   MessageHandlerPair messageHandlerPair(messageId, handler); 
   
   pair<map<unsigned short, MessageHandler>::iterator, bool> pairIterator =
     messageHandlerList_.insert(messageHandlerPair);
                                                                                                         
   if (pairIterator.second)
   {
      return true;
   }//end if
   else
   {
      return false;
   }//end else
}//end add


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Remove the mapping between message Id and handler function
// Design:
//-----------------------------------------------------------------------------
void MessageHandlerList::remove(unsigned short messageId)
{
   ACE_GUARD (ACE_Thread_Mutex, ace_mon, listMutex_);
   messageHandlerList_.erase(messageId);
}//end remove


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return a handler for a given message Id. 
// Design: Returns default handler if message Id could not be found
//-----------------------------------------------------------------------------
const MessageHandler& MessageHandlerList::find(unsigned short messageId)
{
   ACE_GUARD_RETURN (ACE_Thread_Mutex, ace_mon, listMutex_, currentDefaultHandler_);
 
   MessageHandlerRegistryType::iterator iterate = messageHandlerList_.find(messageId);
   if (iterate != messageHandlerList_.end())
   {
      return (*iterate).second;
   }//end if
   else
   {
      return currentDefaultHandler_;
   }//end else
}//end find


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Allows applications to specify a new default message handler 
//              over the one provided by the MsgMgr
// Design:
//-----------------------------------------------------------------------------
void MessageHandlerList::setDefault(MessageHandler& defaultHandler)
{
   ACE_GUARD (ACE_Thread_Mutex, ace_mon, listMutex_);
   currentDefaultHandler_ = defaultHandler;
}//end setDefault


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Restore the default message handler
// Design:
//-----------------------------------------------------------------------------
void MessageHandlerList::restoreDefault()
{
   ACE_GUARD (ACE_Thread_Mutex, ace_mon, listMutex_);

   if(currentDefaultHandler_ != defaultHandler_)
   {
      currentDefaultHandler_ = defaultHandler_;
   }//end if
}//end restoreDefault


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: List all mappings between message Id and handler functions
// Design:
//-----------------------------------------------------------------------------
void MessageHandlerList::listAllMessageHandlers()
{
   ACE_GUARD (ACE_Thread_Mutex, ace_mon, listMutex_);

   ostringstream ostr;
   ostr << "List of active message handlers: " << endl;

   MessageHandlerRegistryType::iterator iterate;
   for (iterate = messageHandlerList_.begin(); iterate != messageHandlerList_.end(); iterate++)
   {
      ostr << iterate->first << endl;
   }//end for

   STRACELOG(DEBUGLOG, MSGMGRLOG, ostr.str().c_str());

}//end listAllMessageHandlers


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MessageHandlerList::toString()
{
   ACE_GUARD_RETURN (ACE_Thread_Mutex, ace_mon, listMutex_, "Guard Error");

   ostringstream ostr;
   ostr << "List of active message handlers: " << endl;

   MessageHandlerRegistryType::iterator iterate;
   for (iterate = messageHandlerList_.begin(); iterate != messageHandlerList_.end(); iterate++)
   {
      ostr << iterate->first << endl;
   }//end for

   return ostr.str().c_str();
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Default message handler
// Design:
//-----------------------------------------------------------------------------
int MessageHandlerList::defaultMessageHandler(MessageBase* messagePtr)
{
   // Do some dummy assignment to prevent unused variable warning
   MessageBase* messagePtrTmp __attribute__ ((unused)) = messagePtr;

   TRACELOG(WARNINGLOG, MSGMGRLOG, "Default message handler invoked, doing nothing",0,0,0,0,0,0);
   return 1;
}//end defaultMessageHandler


//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

