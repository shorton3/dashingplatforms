/******************************************************************************
*
* File name:   MessageBuffer.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include "netinet/in.h"

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "MessageBuffer.h"

#include "platform/logger/Logger.h"

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
MessageBuffer::MessageBuffer(unsigned short bufferSize, bool performNetworkConversion)
   : maxBufferLength_(bufferSize),
     performNetworkConversion_(performNetworkConversion)
{
   if ( bufferSize != 0 )
   {
      if ( (bufferPtr_ = new unsigned char[bufferSize]) == NULL )
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot allocate memory for buffer size.",0,0,0,0,0,0);
      }//end if
      else
      {
         memset(bufferPtr_, 0, maxBufferLength_);
         deserializeFromPtr_ = bufferPtr_;
         bufferInsertPtr_ = bufferPtr_;
      }//end else
   }//end if
   else
   {
      bufferPtr_ = NULL;
      deserializeFromPtr_ = NULL;
      bufferInsertPtr_ = NULL;
   }//end else
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description:
// Design: 
//-----------------------------------------------------------------------------
MessageBuffer::MessageBuffer(unsigned char* bufferPtr, unsigned short bufferSize, bool performNetworkConversion)
   : maxBufferLength_(bufferSize),
     performNetworkConversion_(performNetworkConversion)
{
   if ( bufferSize != 0 )
   {
      bufferPtr_ = bufferPtr;
      deserializeFromPtr_ = bufferPtr_;
      bufferInsertPtr_ = bufferPtr_ + bufferSize;
   }//end if
   else
   {
      bufferPtr_ = NULL;
      deserializeFromPtr_ = NULL;
      bufferInsertPtr_ = NULL;
   }//end else
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Copy Constructor
// Description:
// Design:      Copy Constructor cannot be valid unless OPMBase values are 
//              copied as well.
//-----------------------------------------------------------------------------
//MessageBuffer::MessageBuffer(const MessageBuffer& rhs)
//{
//   bufferPtr_ = rhs.bufferPtr_;
//   deserializeFromPtr_ = rhs.deserializeFromPtr_;
//   bufferInsertPtr_ = rhs.bufferInsertPtr_;
//}//end copy constructor
 

//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MessageBuffer::~MessageBuffer()
{
   delete [] bufferPtr_;
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: Assignment operator
// Description:
// Design:      Assignment operator cannot be valid unless OPMBase values are
//              copied as well.
//-----------------------------------------------------------------------------
//MessageBuffer& MessageBuffer::operator=( const MessageBuffer& rhs)
//{
//   delete bufferPtr_;
//   bufferPtr_ = rhs.bufferPtr_;
//   deserializeFromPtr_ = rhs.deserializeFromPtr_;
//   bufferInsertPtr_ = rhs.bufferInsertPtr_;
//   return *this;
//}//end assignment operator


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: OPMBase static initializer method for bootstrapping the objects
// Design: NOTE: In this class, the initializer will be the message buffer
//         size to create.
//-----------------------------------------------------------------------------
OPMBase* MessageBuffer::initialize(int initializer)
{
   // The 'int' initializer is actually the address of a MessageBufferInitializer
   // structure, so cast, and then let's create the MessageBuffer
   MessageBufferInitializer* bufferInitializer = (MessageBufferInitializer*) initializer;

   return (new MessageBuffer(bufferInitializer->bufferSize, bufferInitializer->performNetworkConversion)); 
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: OPMBase clean method gets called when the object gets released
//              back into its pool
// Design:
//-----------------------------------------------------------------------------
void MessageBuffer::clean()
{
   clearBuffer();
}//end clean


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to return the encapsulated buffer
// Design:
//-----------------------------------------------------------------------------
unsigned char* MessageBuffer::getBuffer()
{
   return deserializeFromPtr_;
}//end getBuffer


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the size of the encapsulated buffer contents
// Design:
//-----------------------------------------------------------------------------
unsigned int MessageBuffer::getBufferLength()
{
   return (bufferInsertPtr_ - deserializeFromPtr_);
}//end getBufferLength


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the max buffer length
// Design:
//-----------------------------------------------------------------------------
unsigned int MessageBuffer::getMaxBufferLength()
{
   return maxBufferLength_;
}//end getMaxBufferLength


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to control the insertion position into the buffer
// Design:
//-----------------------------------------------------------------------------
void MessageBuffer::setInsertPosition(unsigned short nBytes)
{
   bufferInsertPtr_ = bufferPtr_ + nBytes;
}//end setInsertPosition


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to enable/disable Network bit-order conversion 
// Design:      Needed for control when OPM is creating the MessageBuffer
//-----------------------------------------------------------------------------
void MessageBuffer::setNetworkConversion(bool performNetworkConversion)
{
   performNetworkConversion_ = performNetworkConversion;
}//end setNetworkConversion


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to clear out the buffer
// Design:
//-----------------------------------------------------------------------------
void MessageBuffer::clearBuffer()
{
   if (bufferPtr_)
   {
      memset(bufferPtr_, 0, maxBufferLength_);
   }//end if
   deserializeFromPtr_ = bufferPtr_;
   bufferInsertPtr_ = bufferPtr_;
}//end clearBuffer


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to assign the contents of the buffer
// Design:
//-----------------------------------------------------------------------------
void MessageBuffer::assignBuffer(unsigned char* bufferPtr, unsigned short bufferSize)
{
   maxBufferLength_ = bufferSize;
   bufferPtr_ = bufferPtr;
   deserializeFromPtr_ = bufferPtr_;

   if (bufferSize != 0 )
   {
      bufferInsertPtr_ = bufferPtr_ + bufferSize;
   }//end if
   else
   {
      bufferInsertPtr_ = bufferPtr_;
   }//end else
}//end assignBuffer


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Method to assign empty contents to the buffer
// Design:
//-----------------------------------------------------------------------------
void MessageBuffer::assignEmptyBuffer(unsigned char* bufferPtr, unsigned short maxBufferSize)
{
   maxBufferLength_ = maxBufferSize;
   bufferPtr_ = bufferPtr;
   deserializeFromPtr_ = bufferPtr_;
   bufferInsertPtr_ = bufferPtr_;
}//end assignEmptyBuffer


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded insertion operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator<< (int intValue)
{
   if ( (bufferInsertPtr_ + sizeof(int)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
         bufferInsertPtr_,sizeof(int),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   int tempInt = 0;
   if (performNetworkConversion_)
   {
      tempInt = htonl(intValue);
   }//end if
   else
   {
      tempInt = intValue;
   }//end else
   memcpy(bufferInsertPtr_, &tempInt, sizeof(int));
   bufferInsertPtr_ += sizeof(int);
   return *this;
}//end insertion operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded insertion operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator<< (unsigned char charValue)
{
   if ( (bufferInsertPtr_ + sizeof(unsigned char)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
         bufferInsertPtr_,sizeof(unsigned char),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   unsigned char tempChar = charValue;
   memcpy(bufferInsertPtr_, &tempChar, sizeof(unsigned char));
   bufferInsertPtr_ += sizeof(unsigned char);
   return *this;
}//end insertion operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded insertion operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator<< (unsigned short shortValue)
{
   if ( (bufferInsertPtr_ + sizeof(unsigned short)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
         bufferInsertPtr_,sizeof(unsigned short),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   unsigned short tempShort = 0;
   if (performNetworkConversion_)
   {
      tempShort = htons(shortValue);
   }//end if
   else
   {
      tempShort = shortValue;
   }//end else
   memcpy(bufferInsertPtr_, &tempShort, sizeof(unsigned short));
   bufferInsertPtr_ += sizeof(unsigned short);
   return *this;
}//end insertion operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded insertion operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator<< (unsigned int uintValue)
{
   if ( (bufferInsertPtr_ + sizeof(unsigned int)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
         bufferInsertPtr_,sizeof(unsigned int),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   unsigned int tempUInt = 0;
   if (performNetworkConversion_)
   {
      tempUInt = htonl(uintValue);
   }//end if
   else
   {
      tempUInt = uintValue;
   }//end else
   memcpy(bufferInsertPtr_, &tempUInt, sizeof(unsigned int));
   bufferInsertPtr_ += sizeof(unsigned int);
   return *this;
}//end insertion operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded insertion operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator<< (string& stringValue)
{
   unsigned char strLength = stringValue.length() + 1;
   if ( (bufferInsertPtr_ + strLength) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
         bufferInsertPtr_,strLength,bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   *bufferInsertPtr_ = strLength;
   bufferInsertPtr_ ++;
   memcpy(bufferInsertPtr_, stringValue.c_str(), strLength);
   bufferInsertPtr_ += strLength;
   return *this;
}//end insertion operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded insertion operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator<< (bool boolValue)
{
   if ( (bufferInsertPtr_ + sizeof(unsigned char)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
         bufferInsertPtr_,sizeof(unsigned char),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   unsigned char tempChar = 0;
   if (boolValue)
   {
      tempChar = 1;
   }//end if
   memcpy(bufferInsertPtr_, &tempChar, sizeof(unsigned char));
   bufferInsertPtr_ += sizeof(unsigned char);
   return *this;
}//end insertion operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded insertion operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator<< (MailboxAddress& mailboxValue)
{
   unsigned short totalSize = 0;

   // Here we do serialization of the Mailbox Address differently based on its
   // Address type (local, distributed, etc.)
   if (mailboxValue.locationType == LOCAL_MAILBOX)
   {
      // For local mailbox addresses, print a warning. Local mailbox addresses
      // should not be used in distributed mailbox communication because the
      // receiving mailbox will not know the IP Address for sending a reply back
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Local type mailbox address passed to MessageBuffer for serialization",0,0,0,0,0,0);

      // In case we really do need to serialize the local mailbox address,
      // just serialize the locationType and the mailbox name
      int tmpLocationType = 0;
      if (performNetworkConversion_)
      {
         tmpLocationType = htonl(mailboxValue.locationType);
      }//end if
      else
      {
         tmpLocationType = mailboxValue.locationType;
      }//end else
      totalSize += sizeof(int);

      unsigned char nameStrLength = mailboxValue.mailboxName.length() + 1;
      totalSize += nameStrLength;

      // Do size checking to make sure that sufficient space is available -before-
      // starting to serialize
      if ( (bufferInsertPtr_ + totalSize) > (bufferPtr_ + maxBufferLength_) )
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
            bufferInsertPtr_,totalSize,bufferPtr_,maxBufferLength_,0,0);
         return *this;
      }//end if

      // Since sufficient space is available, then perform the serialization
      // Now, perform the serialization. First, serialize the TOTAL Size as the first field
      int tempShort = 0;
      if (performNetworkConversion_)
      {
         tempShort = ntohs(totalSize);
      }//end if
      else
      {
         tempShort = totalSize;
      }//end else
      memcpy(bufferInsertPtr_, &tempShort, sizeof(unsigned short));
      bufferInsertPtr_ += sizeof(unsigned short);

      // Serialize the locationType
      memcpy(bufferInsertPtr_, &tmpLocationType, sizeof(int));
      bufferInsertPtr_ += sizeof(int);

      // Serialize the mailbox name
      *bufferInsertPtr_ = nameStrLength;
      bufferInsertPtr_ ++;
      memcpy(bufferInsertPtr_, mailboxValue.mailboxName.c_str(), nameStrLength);
      bufferInsertPtr_ += nameStrLength;
   }//end if
   else if ( (mailboxValue.locationType == DISTRIBUTED_MAILBOX) || 
             (mailboxValue.locationType == GROUP_MAILBOX) ||
             (mailboxValue.locationType == LOCAL_SHARED_MEMORY_MAILBOX) )
   {
      // For distributed mailbox addresses, serialize all of the fields since we
      // may be working with redundant mailboxes or hardware locations, etc.
      int tmpLocationType = 0;
      if (performNetworkConversion_)
      {
         tmpLocationType = htonl(mailboxValue.locationType);
      }//end if
      else
      {
         tmpLocationType = mailboxValue.locationType;
      }//end else
      totalSize += sizeof(int);

      unsigned char nameStrLength = mailboxValue.mailboxName.length() + 1;
      totalSize += nameStrLength;

      unsigned char neidStrLength = mailboxValue.neid.length() + 1;
      totalSize += neidStrLength;

      int tmpAddressType = 0;
      if (performNetworkConversion_)
      {
         tmpAddressType = htonl(mailboxValue.mailboxType);
      }//end if
      else
      {
         tmpAddressType = mailboxValue.mailboxType;
      }//end else
      totalSize += sizeof(int);

      int tmpShelfNumber = 0;
      if (performNetworkConversion_)
      {
         tmpShelfNumber = htonl(mailboxValue.shelfNumber);
      }//end if
      else
      {
         tmpShelfNumber = mailboxValue.shelfNumber;
      }//end else
      totalSize += sizeof(int);

      int tmpSlotNumber = 0;
      if (performNetworkConversion_)
      {
         tmpSlotNumber = htonl(mailboxValue.slotNumber);
      }//end if
      else
      {
         tmpSlotNumber = mailboxValue.slotNumber;
      }//end else
      totalSize += sizeof(int);

      int tmpRedundantRole = 0;
      if (performNetworkConversion_)
      {
         tmpRedundantRole = htonl(mailboxValue.redundantRole);
      }//end if
      else
      {
         tmpRedundantRole = mailboxValue.redundantRole;
      }//end else
      totalSize += sizeof(int);

      char tmpInetAddress[30];
      // NOTE: this call gives the inetAddress as "IPAddress:port" in string form
      mailboxValue.inetAddress.addr_to_string(tmpInetAddress, sizeof(tmpInetAddress));
      unsigned char tmpInetStrLength = strlen(tmpInetAddress) + 1;
      totalSize += tmpInetStrLength;

      // Do size checking to make sure that sufficient space is available -before-
      // starting to serialize
      if ( (bufferInsertPtr_ + totalSize) > (bufferPtr_ + maxBufferLength_) )
      {
         TRACELOG(ERRORLOG, MSGMGRLOG, "Cannot exceed size of buffer: %d %d %d %d",
            bufferInsertPtr_,totalSize,bufferPtr_,maxBufferLength_,0,0);
         return *this;
      }//end if

      // Now, perform the serialization. First, serialize the TOTAL Size as the first field
      int tempShort = 0;
      if (performNetworkConversion_)
      {
         tempShort = ntohs(totalSize);
      }//end if
      else
      {
         tempShort = totalSize;
      }//end else
      memcpy(bufferInsertPtr_, &tempShort, sizeof(unsigned short));
      bufferInsertPtr_ += sizeof(unsigned short);

      // Serialize the locationType
      memcpy(bufferInsertPtr_, &tmpLocationType, sizeof(int));
      bufferInsertPtr_ += sizeof(int);
  
      // Serialize the mailbox Name
      *bufferInsertPtr_ = nameStrLength;
      bufferInsertPtr_ ++;
      memcpy(bufferInsertPtr_, mailboxValue.mailboxName.c_str(), nameStrLength);
      bufferInsertPtr_ += nameStrLength;

      // Serialize the NEID
      *bufferInsertPtr_ = neidStrLength;
      bufferInsertPtr_ ++;
      memcpy(bufferInsertPtr_, mailboxValue.neid.c_str(), neidStrLength);
      bufferInsertPtr_ += neidStrLength;

      // Serialize the mailbox Address type (physical, logical)
      memcpy(bufferInsertPtr_, &tmpAddressType, sizeof(int));
      bufferInsertPtr_ += sizeof(int);

      // Serialize the shelf number
      memcpy(bufferInsertPtr_, &tmpShelfNumber, sizeof(int));
      bufferInsertPtr_ += sizeof(int);

      // Serialize the slot number
      memcpy(bufferInsertPtr_, &tmpSlotNumber, sizeof(int));
      bufferInsertPtr_ += sizeof(int);

      // Serialize the redundancy role
      memcpy(bufferInsertPtr_, &tmpRedundantRole, sizeof(int));
      bufferInsertPtr_ += sizeof(int);

      // Serialize the IP Address and Port Number
      *bufferInsertPtr_ = tmpInetStrLength;
      bufferInsertPtr_ ++;
      memcpy(bufferInsertPtr_, tmpInetAddress, tmpInetStrLength);
      bufferInsertPtr_ += tmpInetStrLength;
   }//end else if
   return *this;
}//end insertion operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded extraction operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator>> (int& intValue)
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer is empty",0,0,0,0,0,0);
      return *this;
   }//end if

   if ( (deserializeFromPtr_ + sizeof(int)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer contents exhausted prematurely: %d %d %d %d",
         deserializeFromPtr_,sizeof(int),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   int tempInt = 0;
   memcpy(&tempInt, deserializeFromPtr_, sizeof(int));
   if (performNetworkConversion_)
   {
      intValue = ntohl(tempInt);
   }//end if
   else
   {
      intValue = tempInt;
   }//end else
   deserializeFromPtr_ += sizeof(int);
   return *this;
}//end extraction operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded extraction operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator>> (unsigned char& charValue)
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer is empty",0,0,0,0,0,0);
      return *this;
   }//end if

   if ( (deserializeFromPtr_ + sizeof(unsigned char)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer contents exhausted prematurely: %d %d %d %d",
         deserializeFromPtr_,sizeof(unsigned char),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   unsigned char tempChar = 0;
   memcpy(&tempChar, deserializeFromPtr_, sizeof(unsigned char));
   charValue = tempChar;
   deserializeFromPtr_ += sizeof(unsigned char);
   return *this;
}//end extraction operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded extraction operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator>> (unsigned short& shortValue)
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer is empty",0,0,0,0,0,0);
      return *this;
   }//end if

   if ( (deserializeFromPtr_ + sizeof(unsigned short)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer contents exhausted prematurely: %d %d %d %d",
         deserializeFromPtr_,sizeof(unsigned short),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   unsigned short tempShort = 0;
   memcpy(&tempShort, deserializeFromPtr_, sizeof(unsigned short));
   if (performNetworkConversion_)
   {
      shortValue = ntohs(tempShort);
   }//end if
   else
   {
      shortValue = tempShort;
   }//end else
   deserializeFromPtr_ += sizeof(unsigned short);
   return *this;
}//end extraction operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded extraction operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator>> (unsigned int& uintValue)
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer is empty",0,0,0,0,0,0);
      return *this;
   }//end if

   if ( (deserializeFromPtr_ + sizeof(unsigned int)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer contents exhausted prematurely: %d %d %d %d",
         deserializeFromPtr_,sizeof(unsigned int),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   unsigned int tempInt = 0;
   memcpy(&tempInt, deserializeFromPtr_, sizeof(unsigned int));
   if (performNetworkConversion_)
   {
      uintValue = ntohl(tempInt);
   }//end if
   else
   {
      uintValue = tempInt;
   }//end else
   deserializeFromPtr_ += sizeof(unsigned int);
   return *this;
}//end extraction operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded extraction operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator>> (string& stringValue)
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer is empty",0,0,0,0,0,0);
      return *this;
   }//end if

   unsigned char strLength = *deserializeFromPtr_;
   if ( (deserializeFromPtr_ +  strLength) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer contents exhausted prematurely: %d %d %d %d",
         deserializeFromPtr_,strLength,bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   deserializeFromPtr_++;
   stringValue = (char *)deserializeFromPtr_;
   deserializeFromPtr_ += strLength;
   return *this;
}//end extraction operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded extraction operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator>> (bool& boolValue)
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer is empty",0,0,0,0,0,0);
      return *this;
   }//end if

   if ( (deserializeFromPtr_ + sizeof(unsigned char)) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer contents exhausted prematurely: %d %d %d %d",
         deserializeFromPtr_,sizeof(unsigned char),bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if

   boolValue = false;
   unsigned char tempChar = 0;
   memcpy(&tempChar, deserializeFromPtr_, sizeof(unsigned char));
   if (tempChar != 0)
   {
      boolValue = true;
   }//end if
   deserializeFromPtr_ += sizeof(unsigned char);
   return *this;
}//end extraction operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded extraction operator for the buffer
// Design:
//-----------------------------------------------------------------------------
MessageBuffer& MessageBuffer::operator>> (MailboxAddress& mailboxValue)
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer is empty",0,0,0,0,0,0);
      return *this;
   }//end if

   // First retrieve the overall length of the MailboxAddress so we can do size checking
   unsigned short tempShort = 0;
   memcpy(&tempShort, deserializeFromPtr_, sizeof(unsigned short));
   unsigned short dataLength = 0;
   if (performNetworkConversion_)
   {
      dataLength = ntohs(tempShort);
   }//end if
   else
   {
      dataLength = tempShort;
   }//end if

   if ( (deserializeFromPtr_ + dataLength) > (bufferPtr_ + maxBufferLength_) )
   {
      TRACELOG(ERRORLOG, MSGMGRLOG, "Buffer contents exhausted prematurely: %d %d %d %d",
         deserializeFromPtr_,dataLength,bufferPtr_,maxBufferLength_,0,0);
      return *this;
   }//end if
   deserializeFromPtr_ += sizeof(unsigned short);

   // Deserialize the Location Type for the Mailbox Address. Local Addresses have
   // less information
   int tempInt = 0;
   memcpy(&tempInt, deserializeFromPtr_, sizeof(int));
   if (performNetworkConversion_)
   {
      mailboxValue.locationType = (MailboxLocationType)ntohl(tempInt);
   }//end if
   else
   {
      mailboxValue.locationType = (MailboxLocationType)tempInt;
   }//end else
   deserializeFromPtr_ += sizeof(int);

   if (mailboxValue.locationType == LOCAL_MAILBOX)
   {
      // For local mailbox addresses, print a warning. Local mailbox addresses
      // should not be used in distributed mailbox communication because the
      // receiving mailbox will not know the IP Address for sending a reply back
      TRACELOG(DEVELOPERLOG, MSGMGRLOG, "Local type mailbox address retrieved from deserialization",0,0,0,0,0,0);

      // Deserialize the mailbox name
      unsigned char strLength = *deserializeFromPtr_;
      deserializeFromPtr_++;
      mailboxValue.mailboxName = (char *)deserializeFromPtr_;
      deserializeFromPtr_ += strLength;
   }//end if
   else if ( (mailboxValue.locationType == DISTRIBUTED_MAILBOX) || 
             (mailboxValue.locationType == GROUP_MAILBOX) ||
             (mailboxValue.locationType == LOCAL_SHARED_MEMORY_MAILBOX) )
   {
      // For Distributed mailbox address, deserialize all of the fields since we
      // may be working with redundant mailboxes or hardware locations, etc.
      
      // Deserialize the mailbox name
      unsigned char strLength = *deserializeFromPtr_;
      deserializeFromPtr_++;
      mailboxValue.mailboxName = (char *)deserializeFromPtr_;
      deserializeFromPtr_ += strLength;

      // Deserialize the NEID
      unsigned char neidStrLength = *deserializeFromPtr_;
      deserializeFromPtr_++;
      mailboxValue.neid = (char*)deserializeFromPtr_;
      deserializeFromPtr_ += neidStrLength;
 
      // Deserialize the Address type
      memcpy(&tempInt, deserializeFromPtr_, sizeof(int));
      if (performNetworkConversion_)
      {
         mailboxValue.mailboxType = (MailboxAddressType)ntohl(tempInt);
      }//end if
      else
      {
         mailboxValue.mailboxType = (MailboxAddressType)tempInt;
      }//end else
      deserializeFromPtr_ += sizeof(int);

      // Deserialize the Shelf Number
      memcpy(&tempInt, deserializeFromPtr_, sizeof(int));
      if (performNetworkConversion_)
      {
         mailboxValue.shelfNumber = ntohl(tempInt);
      }//end if
      else
      {
         mailboxValue.shelfNumber = tempInt;
      }//end else
      deserializeFromPtr_ += sizeof(int);

      // Deserialize the Slot Number
      memcpy(&tempInt, deserializeFromPtr_, sizeof(int));
      if (performNetworkConversion_)
      {
         mailboxValue.slotNumber = ntohl(tempInt);
      }//end if
      else
      {
         mailboxValue.slotNumber = tempInt;
      }//end else
      deserializeFromPtr_ += sizeof(int);

      // Deserialize the Redundant Role
      memcpy(&tempInt, deserializeFromPtr_, sizeof(int));
      if (performNetworkConversion_)
      {
         mailboxValue.redundantRole = (PreferredRedundantRole)ntohl(tempInt);
      }//end if
      else
      {
         mailboxValue.redundantRole = (PreferredRedundantRole)tempInt;
      }//end else
      deserializeFromPtr_ += sizeof(int);

      // Deserialize the Inet Address
      strLength = *deserializeFromPtr_;
      deserializeFromPtr_++;
      mailboxValue.inetAddress.string_to_addr((char*)deserializeFromPtr_);
      deserializeFromPtr_ += strLength;
   }//end else if
   return *this;
}//end extraction operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return whether all of the contents of this buffer have been
//              processed based on the deserializeFromPtr
// Design:
//-----------------------------------------------------------------------------
bool MessageBuffer::areContentsProcessed()
{
   if (deserializeFromPtr_ == bufferInsertPtr_)
   {
      return true;
   }//end if
   else
   {
      return false;
   }//end else
}//end areContentsProcessed


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MessageBuffer::toString()
{
   ostringstream ostr;

   unsigned short messageId = 0;
   memcpy(&messageId, deserializeFromPtr_, sizeof(unsigned short));

   ostr << "MessageBuffer messageId(" << messageId << ") size(" << maxBufferLength_
        << ") performNetworkConversion(";
   if (performNetworkConversion_ == true)
   {
      ostr << "true)";
   }//end if
   else
   {
      ostr << "false)";
   }//end else
   ostr << ends;
   return (ostr.str().c_str());
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

