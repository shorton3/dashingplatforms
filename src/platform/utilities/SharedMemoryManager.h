/******************************************************************************
* 
* File name:   SharedMemoryManager.h 
* Subsystem:   Platform Services 
* Description: Provides access to a single Shared Memory Map Allocator which
*              should be used/shared by all classes in a single process.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_SHARED_MEMORY_MANAGER_H_
#define _PLAT_SHARED_MEMORY_MANAGER_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Malloc_T.h>
#include <ace/MMAP_Memory_Pool.h>
#include <ace/PI_Malloc.h>
#include <ace/Process_Mutex.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "FixCompilerWarning.h"

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
 * SharedMemoryManager provides access to a single Shared Memory Map Allocator which
 * should be used/shared by all classes in a single process.
 * <p>
 * A single shared memory allocator is sufficient for all uses of shared
 * memory. ACE_Allocator_Adapter is a hash map by itself so we can use
 * bind/find for partitioning the different types of data. If we were to
 * use multiple memory maps, then each would have to use a different base
 * address. ACE_MMAP_Memory_Pool / ACE memory mapped file support uses fixed
 * addresses, and unless it is overriden, and we use multiple files, we would
 * need to map each one to a different start address (by passing in the option
 * to ACE_MMAP_Memory_Pool_Options. Of course, the danger with have multiple
 * memory mapped regions (and the reason why WE DON'T DO IT) is that when the
 * memory regions grow and get 'remapped', we would somehow have to guarantee
 * that those memory address regions do not OVERLAP (which is 'mostly'
 * impossible to do). 
 * <p>
 * We create the shared memory mapped region at ACE_DEFAULT_BASE_ADDR, which
 * for Linux corresponds to address 0x80000000. This allocator handles queue
 * growth using the automatic OS exception handling facilities to trap SIGSEGV
 * and perform an ACE::remap.
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

/** Create an Allocator that uses the Position Indepedent Malloc Facility */
typedef ACE_Allocator_Adapter<ACE_Malloc_T <ACE_MMAP_MEMORY_POOL, ACE_Process_Mutex, ACE_PI_Control_Block> > ALLOCATOR;

class SharedMemoryManager
{
   friend class FixCompilerWarning;

   public:

      /** Virtual Destructor */
      virtual ~SharedMemoryManager();

      /**
       * Returns the static instance of the Shared memory mapped allocator.
       * Performs lazy instantiation of the allocator 
       */
      static ALLOCATOR* getAllocator();

   protected:

   private:

      /** Constructor */
      SharedMemoryManager();

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      SharedMemoryManager(const SharedMemoryManager& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      SharedMemoryManager& operator= (const SharedMemoryManager& rhs);

      /**
       * Shared Memory Map allocator
       */
      static ALLOCATOR* shmemAllocator_;

      /**
       * Memory mapped backing store location
       */
      static const char* backingStoreName_;

};

#endif
