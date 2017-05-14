/******************************************************************************
*
* File name:   DbConnection.cpp
* Subsystem:   Platform Services
* Description: Provides an abstract base class for deriving all database-specific
*              connection classes from.
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

#include "DbConnection.h"

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
DbConnection::DbConnection() : userDefinedIdentifierString_("")
{
}//end constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
DbConnection::~DbConnection()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Allows the Application Developer to 'tag' a particular connection
// Design:
//-----------------------------------------------------------------------------
void DbConnection::addUserDefinedIdentifier(string& identifier)
{
   userDefinedIdentifierString_ = identifier;
}//end addUserDefinedIdentifier


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Returns the user-defined identifier data
// Design:
//-----------------------------------------------------------------------------
string& DbConnection::getUserDefinedIdentifier()
{
   return userDefinedIdentifierString_;
}//end getUserDefinedIdentifier


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Clears the user-defined data and resets the internal string to empty
// Design:
//-----------------------------------------------------------------------------
void DbConnection::clearUserDefinedIdentifier()
{
   userDefinedIdentifierString_ = "";
}//end clearUserDefinedIdentifier


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the String'ized form of the class contents 
// Design:     
//-----------------------------------------------------------------------------
string DbConnection::toString()
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

