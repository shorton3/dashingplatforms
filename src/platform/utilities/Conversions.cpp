/******************************************************************************
*
* File name:   Conversions.h
* Subsystem:   Platform Services
* Description: Utility conversion methods.
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

#include <cstring>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "Conversions.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: 
// Design:     
//-----------------------------------------------------------------------------
string Conversions::integer2Base(unsigned long value, const unsigned int base)
{
   string output(""); // accumulate digits for output, initialized clear
   unsigned int digit; // value of single digit
   char c; // output value for digit
  
   // short-cut
   if ( value == 0 )
   {
      return "0";
   }//end if
  
   while ( value > 0 )
   {
      // get least significant digit
      digit = (unsigned int) value % base; 
      // remove digit
      value = value / base; 

      // get output character for digit
      if (digit > 9 ) // check for non numeric
      {
         c = 'A' + digit - 10;
      }//end if
      else
      {
         c = '0' + digit;
      }//end else
      // add char representing digit
      output = c + output; 
   }//end while
   return output;
}//end integer2Base


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
int Conversions::binary2Decimal(char* binaryString)
{
   int b, k, m, n;
   int len, sum = 0;

   len = strlen(binaryString) - 1;
   for(k = 0; k <= len; k++)
   {
      // character to numeric value
      n = (binaryString[k] - '0'); 
      if ((n > 1) || (n < 0))
      {
         //ERROR! BINARY has only 1 and 0!
         return (0);
      }//end if
      for(b = 1, m = len; m > k; m--)
      {
         // 1 2 4 8 16 32 64 ... place-values, reversed here
         b *= 2;
      }//end for
      // sum it up
      sum = sum + n * b;
      // uncomment to show the way this works
      //printf("%d*%d + ",n,b);
   }//end for
   return(sum);
}//end binary2Decimal


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
Conversions::~Conversions()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

