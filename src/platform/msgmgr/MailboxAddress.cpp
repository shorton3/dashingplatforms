/******************************************************************************
*
* File name:   MailboxAddress.cpp
* Subsystem:   Platform Services
* Description: MailboxAddress encapsulates all of the necessary information
*              for identifying the various types of Mailboxes in the system.
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

#include "MailboxAddress.h"

#include "platform/logger/Logger.h"

#include "platform/common/MailboxNames.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Default Unknown ACE Inet Address
const ACE_INET_Addr MailboxAddress::defaultUnknownInetAddress_((unsigned short)UNKNOWN_PORT_NUMBER,
  (const char*)UNKNOWN_IP_ADDRESS);

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxAddress::MailboxAddress() : mailboxType(UNKNOWN_MAILBOX_TYPE),
                     locationType(UNKNOWN_MAILBOX_LOCATION),
                     shelfNumber(UNKNOWN_SHELF_NUMBER),
                     slotNumber(UNKNOWN_SLOT_NUMBER),
                     mailboxName(UNKNOWN_MAILBOX_NAME),
                     neid(UNKNOWN_NEID),
                     inetAddress(defaultUnknownInetAddress_),
                     redundantRole(REDUNDANT_UNKNOWN)
{

//NOTE: Any change to the parameters of the MailboxAddress REQUIRES A 
// CORRESPONDING CHANGE TO THE MESSAGEBUFFER CLASS SINCE THAT CONTROLS
// HOW MAILBOXADDRESS GETS SERIALIZED/DESERIALIZED!!

}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
MailboxAddress::~MailboxAddress()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Reset all member variables
// Design:
//-----------------------------------------------------------------------------
void MailboxAddress::reset()
{
   mailboxType = UNKNOWN_MAILBOX_TYPE;
   locationType = UNKNOWN_MAILBOX_LOCATION;
   shelfNumber = UNKNOWN_SHELF_NUMBER;
   slotNumber = UNKNOWN_SLOT_NUMBER;
   mailboxName = UNKNOWN_MAILBOX_NAME;
   neid = UNKNOWN_NEID;
   inetAddress.set(defaultUnknownInetAddress_);
   redundantRole = REDUNDANT_UNKNOWN;
}//end reset


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Copy Constructor
// Design:
//-----------------------------------------------------------------------------
MailboxAddress::MailboxAddress(const MailboxAddress& rhs)
{
   mailboxType  = rhs.mailboxType;
   locationType = rhs.locationType;
   shelfNumber  = rhs.shelfNumber;
   slotNumber   = rhs.slotNumber;
   mailboxName  = rhs.mailboxName;
   neid         = rhs.neid;
   inetAddress  = rhs.inetAddress;
   redundantRole= rhs.redundantRole;
}//end Copy Constructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Assignment Operator
// Design:
//-----------------------------------------------------------------------------
MailboxAddress& MailboxAddress::operator = ( const MailboxAddress &rhs )
{
   if ( this != &rhs )
   {
      mailboxType  = rhs.mailboxType;
      locationType = rhs.locationType;
      shelfNumber  = rhs.shelfNumber;
      slotNumber   = rhs.slotNumber;
      mailboxName  = rhs.mailboxName;
      neid         = rhs.neid;
      inetAddress  = rhs.inetAddress;
      redundantRole= rhs.redundantRole;
   }//end if
   return *this;
}//end assignment operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Equality Operator
// Design:
//-----------------------------------------------------------------------------
bool MailboxAddress::operator == ( const MailboxAddress &rhs ) const
{
   bool result = ( ( mailboxType == rhs.mailboxType ) &&
            ( locationType == rhs.locationType ) &&
            ( shelfNumber == rhs.shelfNumber ) &&
            ( slotNumber  == rhs.slotNumber ) &&
            ( mailboxName == rhs.mailboxName ) &&
            ( neid == rhs.neid ) &&
            ( inetAddress == rhs.inetAddress ) &&
            ( redundantRole == rhs.redundantRole) );

   // Debugging:
   // cout << "MailboxAddress equality: " << result << endl;

   return result;
}//end equality operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Comparison Operator
// Design:
//-----------------------------------------------------------------------------
bool MailboxAddress::operator< (const MailboxAddress &rhs) const
{
   // READ Item 19 and Item 39 in Scott Meyers Effective STL about how the
   // associative containers determine equivalence (versus equality)

   // Basically, either (stuff < rhs.stuff) must be TRUE or (stuff > rhs.stuff) must be TRUE
   // BUT, not both!! So, we create a string representation for indexing/ordering
   // the MailboxAddress within the Associative containers
 
   char indexingTmpBuffer[30];
   ostringstream indexingStream; 
   inetAddress.addr_to_string(indexingTmpBuffer, sizeof(indexingTmpBuffer));
   indexingStream << mailboxName << locationType << indexingTmpBuffer << ends;

   char rhsIndexingTmpBuffer[30];
   ostringstream rhsIndexingStream;
   inetAddress.addr_to_string(rhsIndexingTmpBuffer, sizeof(rhsIndexingTmpBuffer));
   rhsIndexingStream << rhs.mailboxName << rhs.locationType << rhsIndexingTmpBuffer << ends;

   bool result = (indexingStream.str() < rhsIndexingStream.str());

   // Debugging:
   // if (mailboxName < rhs.mailboxName)
   //    cout << "MailboxAddress comparison: name is less than" << endl;
   // if (locationType < rhs.locationType)
   //    cout << "MailboxAddress comparison: location type is less than" << endl;
   // if (inetAddress < rhs.inetAddress)
   //    cout << "MailboxAddress comparison: inet address is less than" << endl;
   //
   //cout << "MailboxAddress comparison: returns " << result << endl;

   return result;
}//end comparison operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Comparison Operator
// Design:
//-----------------------------------------------------------------------------
bool MailboxAddress::operator<= (const MailboxAddress &rhs) const 
{
   ostringstream ostr;
   ostr << "MailboxAddress <= called for mailboxes (" << mailboxName << ") and (" << rhs.mailboxName << ")" << ends;
   STRACELOG(DEVELOPERLOG, MSGMGRLOG, ostr.str().c_str());
   return false;
}//end comparison operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Comparison Operator
// Design:
//-----------------------------------------------------------------------------
bool MailboxAddress::operator!= (const MailboxAddress &rhs) const
{
   ostringstream ostr;
   ostr << "MailboxAddress <= called for mailboxes (" << mailboxName << ") and (" << rhs.mailboxName << ")" << ends;
   STRACELOG(DEVELOPERLOG, MSGMGRLOG, ostr.str().c_str());
   return false;
}//end comparison operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Comparison Operator
// Design:
//-----------------------------------------------------------------------------
bool MailboxAddress::operator> (const MailboxAddress &rhs) const
{
   ostringstream ostr;
   ostr << "MailboxAddress <= called for mailboxes (" << mailboxName << ") and (" << rhs.mailboxName << ")" << ends;
   STRACELOG(DEVELOPERLOG, MSGMGRLOG, ostr.str().c_str());
   return false;
}//end comparison operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Overloaded Comparison Operator
// Design:
//-----------------------------------------------------------------------------
bool MailboxAddress::operator>= (const MailboxAddress &rhs) const
{
   ostringstream ostr;
   ostr << "MailboxAddress <= called for mailboxes (" << mailboxName << ") and (" << rhs.mailboxName << ")" << ends;
   STRACELOG(DEVELOPERLOG, MSGMGRLOG, ostr.str().c_str());
   return false;
}//end comparison operator


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string MailboxAddress::toString() const
{
   char tmpBuffer[30];
   ostringstream ostr;

   if (locationType == UNKNOWN_MAILBOX_LOCATION)
      ostr << " LocationType=Unknown";
   else if (locationType == LOCAL_MAILBOX)
      ostr << " LocationType=Local";
   else if (locationType == DISTRIBUTED_MAILBOX)
      ostr << " LocationType=Distributed";
   else if (locationType == GROUP_MAILBOX)
      ostr << " LocationType=Group";

   if (mailboxType == UNKNOWN_MAILBOX_TYPE)
      ostr << " AddressType=Unknown";
   else if (mailboxType == PHYSICAL_MAILBOX)
      ostr << " AddressType=Physical";
   else if (mailboxType == LOGICAL_MAILBOX)
      ostr << " AddressType=Logical";

   if (shelfNumber == UNKNOWN_SHELF_NUMBER)
   {
      ostr << " ShelfNumber=Unknown";
   }//end if
   else
   {
      ostr << " ShelfNumber=" << shelfNumber;
   }//end else

   if (slotNumber == UNKNOWN_SLOT_NUMBER)
   {
      ostr << " SlotNumber=Unknown";
   }//end if
   else
   {
      ostr << " SlotNumber=" << slotNumber;
   }//end else

   ostr << " MailboxName=" << mailboxName;

   if (strcmp(neid.c_str(), UNKNOWN_NEID) == 0)
   {
      ostr << " NEID=Unknown";
   }//end if
   else
   {
      ostr << " NEID=" << neid;
   }//end else

   inetAddress.addr_to_string(tmpBuffer, sizeof(tmpBuffer));
   ostr << " InetAddress=" << tmpBuffer;

   if (redundantRole == REDUNDANT_UNKNOWN)
      ostr << " Redundant=Unknown";
   else if (redundantRole == REDUNDANT_STANDBY)
      ostr << " Redundant=Standby";
   else if (redundantRole == REDUNDANT_ACTIVE)
      ostr << " Redundant=Active";
   else if (redundantRole == REDUNDANT_LOADSHARED)
      ostr << " Redundant=Loadshared";
   ostr << " " << ends;
   return ostr.str().c_str();
}//end toString


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Performs a filter matching algorithm based on non-defaulted
//   fields in the Mailbox Address structure
// Design: At least 1 field must be non-default and must match for this to return
//   true; also, for LocalMailbox LocationTypes, we only check the LocationType
//   and the MailboxName.
//-----------------------------------------------------------------------------
bool MailboxAddress::isMatchingAddress(const MailboxAddress& matchCriteria, const MailboxAddress& addressToCheck)
{
   bool isMatching = false;
   // Check each of the matchingCriteria MailboxAddress fields: first, for whether the
   // is non-default, and second, if it is non-default, is it a match??
   if (matchCriteria.locationType != UNKNOWN_MAILBOX_LOCATION)
   {
      if (matchCriteria.locationType != addressToCheck.locationType)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "LocationType is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if
   if (strcmp(matchCriteria.mailboxName.c_str(), UNKNOWN_MAILBOX_NAME) != 0)
   {
      if (strcmp(matchCriteria.mailboxName.c_str(), addressToCheck.mailboxName.c_str()) != 0)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "MailboxName is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if

   // If this is a LocalMailbox type Address, then only the LocationType and MailboxName matter,
   // so we ignore all of the other fields and return.
   if (addressToCheck.locationType == LOCAL_MAILBOX)
   {
      return isMatching;
   }//end if
   
   if (matchCriteria.mailboxType != UNKNOWN_MAILBOX_TYPE)
   {
      if (matchCriteria.mailboxType != addressToCheck.mailboxType)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "MailboxType is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if
   if (matchCriteria.shelfNumber != UNKNOWN_SHELF_NUMBER)
   {
      if (matchCriteria.shelfNumber != addressToCheck.shelfNumber)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "ShelfNumber is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if
   if (matchCriteria.slotNumber != UNKNOWN_SLOT_NUMBER)
   {
      if (matchCriteria.slotNumber != addressToCheck.slotNumber)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "SlotNumber is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if
   if (strcmp(matchCriteria.neid.c_str(), UNKNOWN_NEID) != 0)
   {
      if (strcmp(matchCriteria.neid.c_str(), addressToCheck.neid.c_str()) != 0)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "NEID is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if
   if (matchCriteria.inetAddress != defaultUnknownInetAddress_)
   {
      if (matchCriteria.inetAddress != addressToCheck.inetAddress)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "InetAddress is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if
   if (matchCriteria.redundantRole != REDUNDANT_UNKNOWN)
   {
      if (matchCriteria.redundantRole != addressToCheck.redundantRole)
      {
         TRACELOG(DEVELOPERLOG, MSGMGRLOG, "RedundantRole is specified as a match criteria, but does not match",0,0,0,0,0,0);
         return false;
      }//end if
      else
      {
         isMatching = true;
      }//end else
   }//end if
   return isMatching;
}//end isMatchingAddress


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

