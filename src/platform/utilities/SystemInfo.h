/******************************************************************************
* 
* File name:   SystemInfo.h 
* Subsystem:   Platform Services 
* Description: Provides static access to system wide attributes such as NEID,
*              Shelf Number, Slot Number, etc.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_SYSTEM_INFO_H_
#define _PLAT_SYSTEM_INFO_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

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
 * SystemInfo provides static access to system wide attributes such as NEID,
 * Shelf Number, Slot Number, etc. 
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class SystemInfo
{
   public:

      /** 
       * Return the local card's NEID (Network Element Id).
       * This method reads the Local NEID from the environment the first time
       * it is called. After that, it caches the value and just returns the cached
       * value (so a complete restart is needed if this value needs to change).
       * @returns the 9 digit NEID string, or it returns "000000000" upon failure
       * if it could not be taken from the environment, its length is not 9, or its
       * characters are not digits.
       */
      static const string& getLocalNEID();

      /** Return the local card's shelf number */
      static unsigned int getShelfNumber();

      /** Return the local card's slot number in the shelf */
      static unsigned int getSlotNumber();

      /** Virtual Destructor */
      virtual ~SystemInfo();

   protected:

   private:

      /** Constructor */
      SystemInfo();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      SystemInfo(const SystemInfo& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      SystemInfo& operator= (const SystemInfo& rhs);

      /** Flag to determine if we already retrieved the Local NEID from the environment */
      static bool isNEIDInitialized_;

      /** Static Local NEID string */
      static string localNEIDStr_;

};

#endif
