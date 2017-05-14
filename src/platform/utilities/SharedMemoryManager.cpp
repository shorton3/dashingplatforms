/******************************************************************************
*
* File name:   SharedMemoryManager.cpp
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


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "SharedMemoryManager.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

// Static instance of the shared memory map allocator
ALLOCATOR* SharedMemoryManager::shmemAllocator_ = NULL;

// Static const string of the backing store file name
const char* SharedMemoryManager::backingStoreName_ = "/tmp/platform.backingstore";

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Allocator_Adapter<ACE_Malloc_T<ACE_MMAP_MEMORY_POOL, ACE_Process_Mutex, ACE_PI_Control_Block> >;
template class ACE_Malloc_T<ACE_MMAP_MEMORY_POOL, ACE_Process_Mutex, ACE_PI_Control_Block>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Allocator_Adapter<ACE_Malloc_T<ACE_MMAP_MEMORY_POOL, ACE_Process_Mutex, ACE_PI_Control_Block> >
#pragma instantiate ACE_Malloc_T<ACE_MMAP_MEMORY_POOL, ACE_Process_Mutex, ACE_PI_Control_Block>
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */

//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
SharedMemoryManager::~SharedMemoryManager()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: STATIC
// Description: Return the common shared memory map allocator
// Design:     
//-----------------------------------------------------------------------------
ALLOCATOR* SharedMemoryManager::getAllocator()
{
   // DO WE NEED SOME TYPE OF MUTEX PROTECTION HERE?? CAN 2 Allocators be created
   // in some cases?

   if (shmemAllocator_ != NULL)
   {
      return shmemAllocator_;
   }//end if
   else
   {
      // For Linux, ACE_DEFAULT_BASE_ADDR = 0x80000000
      ACE_MMAP_Memory_Pool_Options options (ACE_DEFAULT_BASE_ADDR,
         ACE_MMAP_Memory_Pool_Options::ALWAYS_FIXED);
      ACE_NEW_RETURN (shmemAllocator_, ALLOCATOR (backingStoreName_,
         backingStoreName_, &options), NULL);
   }//end else
   return shmemAllocator_;
}//end getAllocator


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

