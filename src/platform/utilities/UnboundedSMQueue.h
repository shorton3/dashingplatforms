/******************************************************************************
* 
* File name:   UnboundedSMQueue.h 
* Subsystem:   Platform Services 
* Description: This class extends the ACE Unbounded Queue (for infinite re-growth)
*              to add allocation support for Shared Memory. Items added to this
*              queue get copied to Position Independent shared memory where they
*              become accessible to other processes.
* 
* Name                 Date       Release 
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release 
* 
*
******************************************************************************/

#ifndef _PLAT_UNBOUNDED_SM_QUEUE_H_
#define _PLAT_UNBOUNDED_SM_QUEUE_H_

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <ace/Malloc_Base.h>
#include <ace/Unbounded_Queue.h>

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
 * UnboundedSMQueue extends the ACE Unbounded Queue (for infinite re-growth)
 * to add allocation support for Shared Memory. Items added to this
 * queue get copied to Position Independent shared memory where they
 * become accessible to other processes. 
 * <p>
 * UnboundedSMQueue is built to be the basis for exchanging tracelog messages
 * between processes and for exchanges between MsgMgr mailboxes in separate
 * processes.
 * <p>
 * NOTE: Our Strategy for using Shared Memory is NOT to create/allocate pools of 
 * shared memory objects (which would use Position Independent pointers) that
 * the applications would reference and use. Rather, we create our containers 
 * which enforce concurrency in shared memory (such as this queue), and then
 * we pass process-local objects through the shared memory from one process
 * to another. (ALSO NOTE: creating pools of objects in shared memory is the
 * job of the in-memory database provider)
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

template <class T>
class UnboundedSMQueue : public ACE_Unbounded_Queue<T>
{
   public:
  
      typedef ACE_Unbounded_Queue<T> BASE;

      /** Constructor */
      UnboundedSMQueue(ACE_Allocator* allocator) : ACE_Unbounded_Queue<T> (allocator) { ; }

      /** Virtual Destructor */
      virtual ~UnboundedSMQueue() { ; }

      /** Enqueue Method */
      int enqueue_tail (const T &new_item, ACE_Allocator* allocator)
      {
         this->allocator_ = allocator;
         return BASE::enqueue_tail (new_item);
      }//end enqueue_tail


      /** Dequeue Method */
      int dequeue_head (T &item, ACE_Allocator* allocator)
      {
         this->allocator_ = allocator;
         return BASE::dequeue_head (item);
      }//end dequeue_head


      /** Clear the Contents */
      void delete_nodes (ACE_Allocator* allocator)
      {
         this->allocator_ = allocator;
         BASE::delete_nodes();
      }//end delete_nodes


      /** Return if the queue is empty */
      int is_empty (ACE_Allocator* allocator)
      {
         this->allocator_ = allocator;
         return BASE::is_empty();
      }//end is_empty


   protected:

   private:

      /**
       * Copy Constructor declared private so that default automatic
       * methods aren't used.
       */
      UnboundedSMQueue(const UnboundedSMQueue& rhs);

      /**
       * Assignment operator declared private so that default automatic
       * methods aren't used.
       */
      UnboundedSMQueue& operator= (const UnboundedSMQueue& rhs);

};

#endif
