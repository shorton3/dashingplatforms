/******************************************************************************
* 
* File name:   LoggerSMConfig.h 
* Subsystem:   Platform Services 
* Description: This class handles storing and retrieving the Logger configuration
*              parameters (such as log level) to and from Shared Memory.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_LOGGER_SM_CONFIG_H_
#define _PLAT_LOGGER_SM_CONFIG_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <string>

#include <ace/Process_Mutex.h>

using namespace std;

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "LoggerSMConfigValues.h"

#include "platform/utilities/SharedMemoryManager.h"

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
 * LoggerSMConfig handles storing and retrieving the Logger configuration
 * parameters (such as log level) to and from Shared Memory.
 * <p>
 * LoggerSMConfig uses position independent shared memory to provide access
 * of the configuration parameters to both the LogProcessor server daemon
 * as well as to the Logger client apis that exist as part of each of the
 * application processes.
 * <p>
 * The shared memory configuration parameters are stored inside a 
 * LoggerSMConfigValues structure.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

class LoggerSMConfig
{
   public:

      /** 
       * Constructor
       * @param configName String name of the Logger SM Config value structure used to identify
       *    it in the underlying shared memory map
       * @param coordinatingMutexName Name used to identify the process mutex for other processes
       */
      LoggerSMConfig(const char* configName, const char* coordinatingMutexName);

      /** Virtual Destructor */
      virtual ~LoggerSMConfig();

      /**
       * Create the (or get a reference to an already created) config values structure
       * @returns OK on success; otherwise ERROR
       */
      int initialize();

      /**
       * Get the Log Level configuration values 
       */
      int getLogLevel(int subsystem);

      /**
       * Set the Log Level configuration values.
       * @returns ERROR on failure; otherwise OK
       */
      int setLogLevel(int subsystem, int severityLevel);

      /** 
       * Reset the contents of the shared memory configuration structure to defaults.
       */
      void reset();

      /** 
       * String'ized debugging method
       * @return string representation of the contents of this object
       */
      string toString();

   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      LoggerSMConfig(const LoggerSMConfig& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      LoggerSMConfig& operator= (const LoggerSMConfig& rhs);

      /** Shared memory position independent allocator */
      ALLOCATOR* shmemAllocator_;

      /** ACE Process Mutex for guarding multi-process shared memory access to the config values structure */
      ACE_Process_Mutex coordinatingMutex_;

      /** Name used for unique identification of the config values structure in Shared Memory */
      string configName_;

      /** Pointer to the actual shared memory config values structure */
      LoggerSMConfigValues* loggerSMConfigValues_;

};

#endif
