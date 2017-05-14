/******************************************************************************
* 
* File name:   MessageBuffer.h 
* Subsystem:   Platform Services 
* Description: This class is a wrapper abstraction for the underlying buffer
*              used to serialize / deserialize messages. This class provides
*              convenience mechanisms to aid developers in marshalling messages.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_MESSAGE_BUFFER_H_
#define _PLAT_MESSAGE_BUFFER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MailboxAddress.h"

#include "platform/opm/OPMBase.h"

#include "platform/common/Defines.h"

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
 * MessageBuffer is a wrapper abstraction for the underlying buffer contained 
 * within the message.
 * <p>
 * This class provides convenience mechanisms to aid developers in marshalling
 * and demarshalling messages. It does not support marshalling of 'long' values.
 * By examining /usr/include/bits/types.h and /usr/include/bits/wordsize.h, 
 * we learn that 'long' is a 32 bit type and is the same as an integer, but that
 * this is not consistent across platforms. Thus, for portability, long types
 * should be avoided.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Initializer structure to pass init params for MessageBuffer into the OPM createPool routine */
struct MessageBufferInitializer
{
   unsigned short bufferSize;
   bool performNetworkConversion;
};//end MessageBufferInitializer


class MessageBuffer : public OPMBase
{
   public:

      /** Constructor */
      MessageBuffer(unsigned short bufferSize = MAX_MESSAGE_LENGTH, bool performNetworkConversion = true);

      /** Constructor */
      MessageBuffer(unsigned char* bufferPtr, unsigned short bufferSize, bool performNetworkConversion = true);

      /** Virtual Destructor */
      virtual ~MessageBuffer();

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

      /**
       * Method to return the encapsulated buffer
       */
      unsigned char* getBuffer();

      /**
       * Return the size of the encapsulated buffer contents
       */
      unsigned int getBufferLength();

      /**
       * Return the max buffer length
       */
      unsigned int getMaxBufferLength();

      /**
       * Method to control the insertion position into the buffer
       */
      void setInsertPosition(unsigned short nBytes);

      /**
       * Method to set the Network Conversion mode.
       * @param performNetworkConversion true is network bit conversion should be performed;
       *   otherwise false (for shared memory operations)
       */
      void setNetworkConversion(bool performNetworkConversion);

      /**
       * Method to clear out the buffer
       */
      void clearBuffer();

      /**
       * Method to assign the contents of the buffer
       */
      void assignBuffer(unsigned char* bufferPtr, unsigned short bufferSize);

      /**
       * Method to assign empty contents to the buffer
       */
      void assignEmptyBuffer(unsigned char* bufferPtr, unsigned short maxBufferSize);

      /**
       * Overloaded insertion operators for the buffer
       */
      MessageBuffer& operator<< (int intValue);
      MessageBuffer& operator<< (unsigned char charValue);
      MessageBuffer& operator<< (unsigned short shortValue);
      MessageBuffer& operator<< (unsigned int uintValue);
      MessageBuffer& operator<< (string& stringValue);
      MessageBuffer& operator<< (bool boolValue);
      MessageBuffer& operator<< (MailboxAddress& mailboxValue);

      /**
       * Overloaded extraction operators for the buffer
       */
      MessageBuffer& operator>> (int& intValue);
      MessageBuffer& operator>> (unsigned char& charValue);
      MessageBuffer& operator>> (unsigned short& shortValue);
      MessageBuffer& operator>> (unsigned int& uintValue);
      MessageBuffer& operator>> (string& stringValue);
      MessageBuffer& operator>> (bool& boolValue);
      MessageBuffer& operator>> (MailboxAddress& mailboxValue);

      /**
       * Method to return whether or not this Message Buffer has been completely deserialized
       * based on the position of the deserializeFromPtr
       */
      bool areContentsProcessed();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Copy Constructor 
       */
      MessageBuffer(const MessageBuffer& rhs);

      /**
       * Assignment operator
       */
      MessageBuffer& operator= (const MessageBuffer& rhs);

      /** Default Constructor */
      MessageBuffer();

      /**
       * Pointer to the encapsulated buffer
       */
      unsigned char* bufferPtr_;
   
      /**
       * Cursor pointer to the buffer
       */
      unsigned char* deserializeFromPtr_;
  
      /**
       * Cursor pointer to the buffer
       */
      unsigned char* bufferInsertPtr_;

      /**
       * Maximum buffer size
       */
      unsigned short maxBufferLength_;

      /** Flag to indicate whether network bit conversion should be performed. This should
          be true if the MessageBuffer will be transported through the network stack; otherwise
          it should be false (such as in the case for shared memory transport). */
      bool performNetworkConversion_;

};

#endif
