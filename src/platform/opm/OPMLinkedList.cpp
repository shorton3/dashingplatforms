/******************************************************************************
*
* File name:   OPMLinkedList.cpp
* Subsystem:   Platform Services
* Description: Implements the storage structure that the pooled objects are
*              stored within.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#include "OPMLinkedList.h"

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

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
OPMLinkedList::OPMLinkedList()
   :head_(NULL),tail_(NULL),listSize_(0)
{
}//end Constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
OPMLinkedList::~OPMLinkedList()
{
}//end destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Remove an OPMBase object from the Head (First) of the Linked 
//              List and return the object back to the user
// Design:     
//-----------------------------------------------------------------------------
OPMBase* OPMLinkedList::removeFirst()
{
   //if list is empty, return null
   if (head_ == NULL)
      return NULL;

   //Remove the top OPMBase object by rerouting the head link to point to
   // the second link.
   OPMBase* object = head_;
   head_ = head_->getNext();

   //if the new head is null, that means the list is empty (set tail null)
   if (head_ == NULL)
      tail_ = NULL;
   //else set the 'new' head's previous pointer to NULL 
   else
      head_->setPrev(NULL);

   //remember to null out the remove node's next and previous references
   object->setNext(NULL);
   object->setPrev(NULL);
   listSize_ -= 1;

   return object;
}//end removeFirst


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Remove a specified OPMBase object from the Linked List by 
//              manipulating the previous and next pointers
// Design:     
//-----------------------------------------------------------------------------
OPMBase* OPMLinkedList::remove(OPMBase* object)
{
   //if list is empty, return null
   if (head_ == NULL)
      return NULL;

   //Get the previous and next nodes for the object to remove
   OPMBase* previousNode = object->getPrev();
   OPMBase* nextNode = object->getNext();

   //Start to remove the specified object by hooking up its previous node
   if (previousNode)
      previousNode->setNext(nextNode);
   else if (head_ == object) //Should always be the case
   {
      head_ = nextNode;
   }//end else if
      
   //Hook up the next node's previous pointer to the previous node
   if (nextNode)
      nextNode->setPrev(previousNode);
   else if (tail_ == object) //Should always be the case
   {
      tail_ = previousNode;
   }//end else if

   //remember to null out the remove node's next and previous references
   object->setNext(NULL);
   object->setPrev(NULL);
   listSize_ -= 1;

   return object;
}//remove


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Insert an OPMBase object at the Head (First) of the Linked List
// Design:     
//-----------------------------------------------------------------------------
void OPMLinkedList::insertFirst(OPMBase* object)
{
   //check to see if the list is empty, if so, new object is both head and tail
   if (head_ == NULL)
   {
      tail_ = object;
      head_ = object;
   }//end if
   else
   {
      //set the current head's previous pointer to the new head node
      head_->setPrev(object);

      //new object's next points to current head
      object->setNext(head_);

      //reference to head now assigned to new object
      head_ = object;
   }//end else
   listSize_ += 1;
}//end insertFirst


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Insert an OPMBase object at the Tail (Last) of the Linked List
// Design:     
//-----------------------------------------------------------------------------
void OPMLinkedList::insertLast(OPMBase* object)
{
   //check to see if the list is empty, if so, new object is both head and tail
   if (tail_ == NULL)
   {
      tail_ = object;
      head_ = object;
   }//end if
   else
   {
      //current tail object's next points to new object
      tail_->setNext(object);

      //Set new object's previous pointer to the current tail
      object->setPrev(tail_);

      //reference to tail now assigned to new object
      tail_ = object;
   }//end else
   listSize_ += 1;
}//end insertFirst


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Return the size of the List
// Design:     
//-----------------------------------------------------------------------------
int OPMLinkedList::getListSize()
{
   return listSize_;
}//end getListSize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Check if the list is currently empty
// Design:     
//-----------------------------------------------------------------------------
bool OPMLinkedList::isEmpty()
{
   return (listSize_ == 0);
}//end isEmpty


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Walk the linked list and check to see if the object is a member
// Design:     
//-----------------------------------------------------------------------------
bool OPMLinkedList::containsObject(OPMBase* object)
{
   bool foundMatch = false;

   //if list is empty, return false 
   if (head_ == NULL)
      return foundMatch;

   OPMBase* temp = head_;
   if (temp == object)
      foundMatch = true;
   else
   {
      while (!temp->isNextNull())
      { 
         temp = temp->getNext();
         if (temp == object)
         {
            foundMatch = true;
            break;
         }//end if
      }//end while
   }//end else
   return foundMatch;
}//end containsObject


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Display the contents of the list
// Design:     
//-----------------------------------------------------------------------------
string OPMLinkedList::toString()
{
   string returnStr = "OPMLinkedList contents: ";
   //traverse the list and call each OPMBase's toString method
   OPMBase* temp = head_;
   returnStr += temp->toString();
   while (!temp->isNextNull())
   {
      temp = temp->getNext();
      returnStr += ",";
      returnStr += temp->toString();
   }//end while
   returnStr += " ";
   return returnStr;
}//end displayList


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Delete the contents of the list - does not delete the list
//              objects, but rather deletes the list references.
// Design:     
//-----------------------------------------------------------------------------
void OPMLinkedList::deleteListContents()
{
   OPMBase* temp = head_;
   //traverse the list and free the next pointers
   while (!head_->isNextNull())
   {
      head_ = head_->getNext();
      temp->setNext(NULL);
      temp->setPrev(NULL);
      delete temp;
      temp = head_;
   }//end while
   delete head_;
   head_ = NULL;
   tail_ = NULL;
}//end deleteListContents

//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------
