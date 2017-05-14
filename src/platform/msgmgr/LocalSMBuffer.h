/******************************************************************************
* 
* File name:   LocalSMBuffer.h 
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

#ifndef _PLAT_LOCAL_SM_BUFFER_H_
#define _PLAT_LOCAL_SM_BUFFER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Based_Pointer_T.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "platform/common/Defines.h"

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
 * LocalSMBuffer class is a wrapper for the MessageBuffer's raw unsigned
 * char buffer so that it can be passed through the shared memory
 * queue with a position independent pointer. 
 * <p>
 * LocalSMBuffer assumes that all shared memory passed message will be
 * of length MAX_MESSAGE_LENGTH.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

struct LocalSMBuffer : public OPMBase
{
   /** Raw unsigned char message buffer from the MessageBuffer class */
   unsigned char buffer[MAX_MESSAGE_LENGTH];

   /** Position Independent ACE Pointer to the log Message payload string. NOTE: there is no allocated memory here */
   ACE_Based_Pointer_Basic<unsigned char> bufferPI;

   /** Priority Level of the Message. Default is 0. */
   unsigned int priorityLevel;

   /** Version number of the Message */
   unsigned int versionNumber;

   /** Buffer Length / sizeof the data */
   unsigned int bufferLength;

   /** Constructor */
   LocalSMBuffer();

   /** Virtual Destructor */
   virtual ~LocalSMBuffer();

   /** Copy Constructor */ 
   LocalSMBuffer(const LocalSMBuffer& rhs);

   /** Reset all data members */
   void reset(void);

   /** Overloaded Assignment Operator*/
   LocalSMBuffer& operator= (const LocalSMBuffer& rhs);

   /**
    * OPMBase static initializer method for bootstrapping the objects
    * @param initializer Here the initializer will be the message buffer
    *    size to create. This assumes that we will have an OPM object pool
    *    for each size of Message Buffer that we need.
    */
   static OPMBase* initialize(int initializer);

   /** OPMBase clean method gets called when the object gets released back
       into its pool */
   void clean();

};

#endif
