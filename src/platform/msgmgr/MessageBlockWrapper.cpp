/******************************************************************************
*
* File name:   MessageBlockWrapper.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageBlockWrapper.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageBlockWrapper::MessageBlockWrapper(int blockSize)
                    :ACE_Message_Block(blockSize)
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageBlockWrapper::~MessageBlockWrapper()
{
   // Release the Message_Block so that it can be reference counted and disposed of
   // NOTE: this may NOT work, so do NOT ever let OPM delete these objects!!
   // Only allow OPM to grow these objects.
   release();
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: OPMBase static initializer method for bootstrapping the objects
// Design:
//-----------------------------------------------------------------------------
OPMBase* MessageBlockWrapper::initialize(int initializer)
{
   MessageBlockWrapper* messageBlock = NULL;
   if (initializer == 0)
   {
      messageBlock = new MessageBlockWrapper();
   }//end if
   else
   {
      messageBlock = new MessageBlockWrapper(initializer);
   }//end else
   return messageBlock;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: OPMBase clean method gets called when the object gets released 
//              back into its pool
// Design:
//-----------------------------------------------------------------------------
void MessageBlockWrapper::clean()
{
}//end clean


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overridden ACE_Message_Block Method
// Design:
//-----------------------------------------------------------------------------
void MessageBlockWrapper::base(char *data, size_t size)
{
   ACE_Message_Block::base(data,size);
}//end base


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overridden ACE_Message_Block Method
// Design:
//-----------------------------------------------------------------------------
char* MessageBlockWrapper::base (void) const
{
   return ACE_Message_Block::base();
}//end base


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overridden ACE_Message_Block Method
// Design:
//-----------------------------------------------------------------------------
void MessageBlockWrapper::msg_priority(unsigned long priority)
{
   ACE_Message_Block::msg_priority(priority);
}//end msg_priority


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MessageBlockWrapper::toString()
{
   string s = "";
   return (s);
}//end toString


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

