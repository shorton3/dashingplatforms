/******************************************************************************
*
* File name:   LocalSMBuffer.cpp
* Subsystem:   Platform Services
* Description: This class is a wrapper for the MessageBuffer's raw unsigned
*              char buffer so that it can be passed through the shared memory
*              queue with a position independent pointer.
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

#include <cstdio>
#include <cstring>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LocalSMBuffer.h"

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
LocalSMBuffer::LocalSMBuffer()
              : priorityLevel(0),
                versionNumber(0),
                bufferLength(0)
{
   for (unsigned int i = 0; i < sizeof(buffer); i++)
   { 
      buffer[i] = '\0';
   }//end for
   bufferPI = buffer;

   // Set some dummy identifier OPMBase identifier string
   OPMBase::setObjectTypeStr("LocalSMBuffer"); 
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
LocalSMBuffer::~LocalSMBuffer()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: Copy Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
LocalSMBuffer::LocalSMBuffer( const LocalSMBuffer& rhs )
              : OPMBase(),
                priorityLevel(rhs.priorityLevel),
                versionNumber(rhs.versionNumber),
                bufferLength(rhs.bufferLength)
{
   ACE_OS::memcpy(buffer, rhs.buffer, sizeof(rhs.buffer));
   bufferPI = buffer;
}//end copy constructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reset the data members
// Design:
//-----------------------------------------------------------------------------
void LocalSMBuffer::reset()
{
   for (unsigned int i = 0; i < sizeof(buffer); i++)
   {
      buffer[i] = '\0';
   }//end for
   bufferPI = buffer;
   priorityLevel = 0;
   versionNumber = 0;
   bufferLength = 0;
}//end reset


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded assignment operator
// Design:
//-----------------------------------------------------------------------------
LocalSMBuffer& LocalSMBuffer::operator= ( const LocalSMBuffer& rhs)
{
   if (this != &rhs)
   {
      ACE_OS::memcpy(buffer, rhs.buffer, sizeof(rhs.buffer));
      bufferPI = buffer;
      priorityLevel = rhs.priorityLevel;
      versionNumber = rhs.versionNumber;
      bufferLength = rhs.bufferLength;
   }//end if
   return *this;
}//end assignment operator


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: OPMBase static initializer method for bootstrapping the objects
// Design: 
//-----------------------------------------------------------------------------
OPMBase* LocalSMBuffer::initialize(int initializer)
{
   // remove compiler warning
   initializer = 0;

   return (new LocalSMBuffer());
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: OPMBase clean method gets called when the object gets released
//              back into its pool
// Design:
//-----------------------------------------------------------------------------
void LocalSMBuffer::clean()
{
   reset();
}//end clean


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

