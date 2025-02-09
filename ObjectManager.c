//-----------------------------------------
// REMARKS: The purpose of this program is to provide
//          an implementation for the ObjectManager.h
//          interface, and build a memory management
//          system.
//-----------------------------------------

#include "ObjectManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// node Struct
typedef struct NODE Node;
struct NODE
{
  int memStartIndex; //offset index
  ulong memSize;
  int objReferenceCount;
  Ref objReferenceID;
  Node* next;
};

// index linked list struct
typedef struct INDEX Index;
struct INDEX
{
  Node* top;
};

//---------------------------------------------------
// global variables needed for Memory Pool management
//---------------------------------------------------

static int numObjMngrs = 0; // # of object managers initialised
static uchar* activeBuffer; // two buffers for double buffering
static uchar* inactiveBuffer;
static Ref referenceID; //keeps track of the highest id that has been given out
static int nextAvailableIndex; // next available index in the buffer
static Index* indexing; // index to keep track of objects

//---------------------
// FUNCTION PROTOTYPES
//---------------------

// node struct functions
static Node* makeNode(ulong memSize);
static void destroyNode(Node* aNode);
static void checkNode(Node* aNode);

// index struct functions
static Index* makeIndex();
static void destroyIndex(Index* anIndex);
static void checkIndex(Index* anIndex);

// garbage collection related functions
static int garbageExists();
static void compact();
static void copyActToNonact();
static void swapBuffers();
static void updateIndex();
static void insertAtEnd(Node* aNode);

// find the node by the given ref
static Node* findNode(Ref ref);

//------------------------------------------------------
// initPool
//
// PURPOSE: initialises the object manager to prepare all
//          necessary resources to implement the memory
//          management system.
//
// (No return type or input/output parameters)
//------------------------------------------------------
void initPool()
{
  // initialise an object manager only if there isn't an object
    //manager initialised already
  if(numObjMngrs == 0)
  {
    // initialise all global variables
    activeBuffer = (uchar*)(malloc(sizeof(uchar) * MEMORY_SIZE));
    inactiveBuffer = (uchar*)(malloc(sizeof(uchar) * MEMORY_SIZE));
    assert(activeBuffer != NULL);
    assert(inactiveBuffer != NULL);
    referenceID = 1;
    nextAvailableIndex = 0; //starting at index 0
    indexing = makeIndex();
    checkIndex(indexing);
    numObjMngrs++;
  }
  else
  {
    printf("\nThere is an Object Manager initialised already!\n");
  }
} // end of initPool()

//------------------------------------------------------
// destroyPool
//
// PURPOSE: cleans up all the memory being used by the
//          Object manager.
//
// (No return type or input/output parameters)
//------------------------------------------------------
void destroyPool()
{
  // if there is actually an object manager initialised that needs to cleaned up
  if(numObjMngrs != 0)
  {
    numObjMngrs--;
    // check all resources are valid before destroying
    assert(activeBuffer != NULL);
    assert(inactiveBuffer != NULL);
    checkIndex(indexing);
    // clean up memory being used
    free(activeBuffer);
    activeBuffer = NULL;
    free(inactiveBuffer);
    inactiveBuffer = NULL;
    destroyIndex(indexing);
  }
} // end of destroyPool()

//------------------------------------------------------
// insertObject
//
// PURPOSE: This function trys to allocate a block of given
//          size from our buffer. It will fire the garbage
//          collector as required.
//
// INPUT PARAMETERS:
// size - the amount of bytes being requested for allocation
//        for an object
//
// RETURN:
// if memory is allocated successfully, it returns the reference
// number for the block of memory allocated for the object.
// Otherwise, it returns NULL_REF (0)
//------------------------------------------------------
Ref insertObject(ulong size)
{
  assert(numObjMngrs != 0);
  // size cannot be negative
  assert(size >= 0);
  Ref returnRef = NULL_REF;

  if(numObjMngrs != 0)
  {
    //nothing is allocated if 0 bytes, or more than total memory
    //available is requested
    if (size > 0 && size <= MEMORY_SIZE)
    {
      // if there is room available on the buffer for the requested amount
      if(size <= (MEMORY_SIZE - nextAvailableIndex))
      {
        //allocate memory and update index
        returnRef = referenceID;
        Node* insertNode = makeNode(size);
        checkNode(insertNode);
        insertAtEnd(insertNode);
      }
      else
      {
        //space not available, fire garbage collection
        compact();
        // check if enough space available even after garbage collecting
        if(size <= (MEMORY_SIZE - nextAvailableIndex))
        {
          // allocate if room created after garbage collection
          returnRef = referenceID;
          Node* insertNode = makeNode(size);
          checkNode(insertNode);
          insertAtEnd(insertNode);
        }
      }
    }
    checkIndex(indexing);
  }
  else
  {
    printf("There are no object managers initialised. Initialise an object manager to gain access to memory.\n");
  }
  return returnRef;
} // end of insertObject()

//------------------------------------------------------
// insertAtEnd
//
// PURPOSE: inserts a Node at the end of the object manager's
//          index (i.e. the linked list)
//
// INPUT PARAMETERS:
// aNode - the node being inserted in the index
//------------------------------------------------------
static void insertAtEnd(Node* aNode)
{
  checkIndex(indexing);
  checkNode(aNode);

  // case 1: index is empty
  if(indexing->top == NULL)
  {
    indexing->top = aNode;
  }   
  else
  {
    // case 2: index not empty
    Node* curr = indexing->top;
    Node* prev = NULL;
    // iterate index to get to the end
    while(curr != NULL)
    {
      prev = curr;
      curr = curr->next;
    }
    // insert node
    prev->next = aNode;
  }
  checkIndex(indexing);
} //end of insertAtEnd()

//------------------------------------------------------
// compact
//
// PURPOSE: this function initiates garbage collection.
//          Reference Counting technique is used as a way
//          of determining which objects are garbage.
//          Defragmentation is handled by double buffering.
//          To keep code clean, this function calls various
//          other private functions to carry out the
//          individual tasks.
//------------------------------------------------------
static void compact()
{
  checkIndex(indexing);
  printf("\nGarbage collector statistics:\n");

  //step1: copy nongarbage from active to inactive buffer
  copyActToNonact();

  //step2: swap buffers
  swapBuffers();

  //step3: update index keeping track of objects
  updateIndex();
  checkIndex(indexing);

}// end of compact()

//------------------------------------------------------
// updateIndex
//
// PURPOSE: when garbage collection is initiated, this function
//          helps update the index linked list to clean up
//          and remove objects with a ref count of 0 after
//          the buffers have been dealt with already.
//
//------------------------------------------------------
static void updateIndex()
{
  checkIndex(indexing);

  // clean until index has objects with a ref count of 0
  while(garbageExists() == 1)
  {
    Node* curr = indexing->top;
    Node* prev = NULL;
    // finding the first occurence of a node with 0 references
    while(curr->objReferenceCount != 0)
    {
      prev = curr;
      curr = curr->next;
    }
    // if there is only a single node in the list
    if (indexing->top->next == NULL)
    {
      indexing->top = NULL;
    }
    else
    {
      if(prev == NULL) //removing from front
      {
        indexing->top = curr->next;
      }
      else //removing from back or middle
      {
        prev->next = curr->next;
      }
    }
    checkIndex(indexing);
  }
  checkIndex(indexing);
}// end of updateIndex()

//------------------------------------------------------
// copyActToNonact
//
// PURPOSE: when garbage collection is initiated, this function
//          helps with copying non garbage objects from the current
//          active buffer to the inactive buffer. During this
//          process we keep track of the garbage collection
//          statistics and print them to the console. We make
//          use of the index linked list ot see which objects
//          are garbage and which aren't.
//------------------------------------------------------
static void copyActToNonact()
{
  checkIndex(indexing);
  Node* curr = indexing->top;
  int newStartInd = 0; // new start index for an object in the inactive buffer
  int sizeTracker = 0; // size tracker for inactive buffer and where next avail index is
  int numObjects = 0;  // non garbage objects detected
  ulong numBytes = 0;  // bytes in use
  ulong numBytesCollected = 0; // bytes collected by GC

  // iterating index
  while(curr != NULL)
  {
    // we copy non garbage only
    if(curr->objReferenceCount != 0)
    {
      numObjects++;
      numBytes = numBytes + curr->memSize;
      //copy from active to inactive
      for(int i = curr->memStartIndex; i < (curr->memStartIndex + curr->memSize); i++)
      {
        inactiveBuffer[sizeTracker] = activeBuffer[i];
        sizeTracker++; 
      }
      // update the object's new offset
      curr->memStartIndex = newStartInd;
      newStartInd = newStartInd + curr->memSize;
    }
    else
    {
      // if garbage we collect it and do not copy over
      numBytesCollected = numBytesCollected + curr->memSize;
    }
    curr = curr->next;
  }

  // update global variable for object manager
  nextAvailableIndex = sizeTracker;     

  // printing garbage collection statistics
  printf("Objects: %d   Bytes in Use: %lu   Freed: %lu\n", numObjects, numBytes, numBytesCollected);
  checkIndex(indexing);
}// end of copyActToNonact()

//------------------------------------------------------
// swapBuffers
//
// PURPOSE: when garbage collection is initiated, this function
//          helps with swapping the active and inactive buffers
//          after we have already copied non garbage from current
//          active buffer to the inactive buffer.
//------------------------------------------------------
static void swapBuffers()
{
  assert(activeBuffer != NULL);
  assert(inactiveBuffer != NULL);

  //swapping
  uchar* temp = activeBuffer;
  activeBuffer = inactiveBuffer;
  inactiveBuffer = temp;
  assert(activeBuffer != NULL);
  assert(inactiveBuffer != NULL);
}// swapBuffers()

//------------------------------------------------------
// garbageExists
//
// PURPOSE: this function checks if there are any objects
//          that have gone out of scope (i.e. garbage)
//
// RETURN:
// returns 0 if no garbage and 1 otherwise
//------------------------------------------------------
static int garbageExists()
{
  checkIndex(indexing);
  int present = 0;

  // iterate the index to see if there are any objects
    //with a ref count of 0. we exit as soon as we find one
  Node* curr = indexing->top;
  while(curr != NULL && present != 1)
  {
    if(curr->objReferenceCount == 0)
    {
      present = 1;
    }
    curr = curr->next;
  }
  checkIndex(indexing);
  return present;
} // end of garbageExists()

//------------------------------------------------------
// dumpPool()
//
// PURPOSE: This function traverses the index and prints the
//          info in each non-garbage entry corresponding to
//          a block of allocated memory.
//------------------------------------------------------
void dumpPool()
{
  assert(numObjMngrs != 0);
  if(numObjMngrs != 0)
  {
    checkIndex(indexing);
    //keeps track of the ith non-garbage object we found
    int counter = 1;

    Node* curr = indexing->top;
    while(curr != NULL)
    {
      // print info if object is still in scope
      if(curr->objReferenceCount != 0)
      {
        printf("\nObject #%d Info:\n", counter);
        counter++;
        printf("Starting index - %d\n", curr->memStartIndex);
        printf("Starting Address - %p\n", &(activeBuffer[curr->memStartIndex]));
        printf("Reference ID - %lu\n", curr->objReferenceID);
        printf("Size - %lu\n", curr->memSize);
        printf("Reference Count - %d\n", curr->objReferenceCount);
      }
      curr = curr->next;
    }
    checkIndex(indexing);
  }
  else
  {
    printf("No object manager initialised. Nothing available in memory pool to dump!\n");
  }
} // end of dumpPool()

//------------------------------------------------------
// retrieveObject
//
// PURPOSE: returns a pointer to the object in memory being
//          requested given by the reference id
//
// INPUT PARAMETERS:
// ref - the reference id for the object to which we want
//       the pointer for
//
// RETURN:
// a void pointer to where the x amount of memory was allocated
// for the object.
//------------------------------------------------------
void* retrieveObject(Ref ref)
{
  assert(numObjMngrs != 0);
  void* ptr = NULL;

  if(numObjMngrs != 0)
  {
    // reference being used hasn't been given out yet
    assert(ref < referenceID);
    assert(ref != NULL_REF);
    // procced if ref is not null
    if(ref != NULL_REF)
    {
      // find the node in the index with the ref of interest
      Node* target = findNode(ref);
      // if the node was found and it is not out of scope
      if(target != NULL && target->objReferenceCount != 0)
      {
        checkNode(target);
        ptr = &(activeBuffer[target->memStartIndex]);
        assert(ptr != NULL);
      }
    }
    else
    {
      assert(ptr == NULL);
    }
  }
  return ptr;
} // end of retrieveObject

//------------------------------------------------------
// findNode
//
// PURPOSE: searches the index for a node with the specific
//          ref id.
//
// INPUT PARAMETERS:
// ref - the reference id we are searching for
//
// RETURN:
// if found, a pointer to the target node, null otherwise
//------------------------------------------------------
static Node* findNode(Ref ref)
{
  assert(ref < referenceID);
  assert(ref != NULL_REF);

  Node* returnNode = NULL;
  if(ref != NULL_REF)
  {
    checkIndex(indexing);
    Node* curr = indexing->top;
    // iterate until we reach the end or find the target ref
    while(curr != NULL && curr->objReferenceID != ref)
    {
      curr = curr->next;
    }
    returnNode = curr;
    checkIndex(indexing);
  }
  return returnNode;

} // end of findNode()

//------------------------------------------------------
// addReference
//
// PURPOSE: updates our index to indicate that we have another
//          reference to the given object
//
// INPUT PARAMETERS:
// ref - reference to the object for which we want to increment
//       number of references
//------------------------------------------------------
void addReference(Ref ref)
{
  assert(numObjMngrs != 0);

  if(numObjMngrs != 0)
  {
    // reference being used hasn't been given out yet
    assert(ref < referenceID);
    assert(ref != NULL_REF);

    if(ref != NULL_REF)
    {
      //find the node of interest
      Node* targetObj = findNode(ref);
      // if node is found and it is still in scope
      if(targetObj != NULL && targetObj->objReferenceCount != 0)
      {
        checkNode(targetObj);
        targetObj->objReferenceCount++;
        checkNode(targetObj);
      }
    }
  }
} //end of addReference

//------------------------------------------------------
// dropReference
//
// PURPOSE: updates our index to indicate that we have lost
//          a reference to the given object
//
// INPUT PARAMETERS:
// ref - reference to the object for which we want to decrement
//       number of references
//------------------------------------------------------
void dropReference(Ref ref)
{
  assert(numObjMngrs != 0);

  if(numObjMngrs != 0)
  {
    // reference being used hasn't been given out yet
    assert(ref < referenceID);
    assert(ref != NULL_REF);

    if(ref != NULL_REF)
    {
      // find the node of interest
      Node* targetObj = findNode(ref);
      // if node is found and it is still in scope
      if(targetObj != NULL && targetObj->objReferenceCount != 0)
      {
        checkNode(targetObj);
        targetObj->objReferenceCount--;
        checkNode(targetObj);
      }
    }
  } 
} // end of dropReference()

//------------------------------------------------------
// makeNode
//
// PURPOSE: creates a new instance of the node struct
//
// INPUT PARAMETERS:
// memSize - The size of the object that the node will represent in
//           the index
//
// RETURN:
// A pointer to the new Node created
//------------------------------------------------------
static Node* makeNode(ulong memSize)
{
  assert(memSize >= 0);
  Node* newNode = (Node*)(malloc(sizeof(Node)));
  assert(newNode != NULL);
  if(newNode != NULL)
  {
    newNode->memStartIndex = nextAvailableIndex;
    // update global variable for next available index in the buffer
    nextAvailableIndex = nextAvailableIndex + memSize;
    newNode->memSize = memSize;
    newNode->objReferenceCount = 1;
    newNode->objReferenceID = referenceID;
    // update global variable for reference id to avoid duplicate ref ids
    referenceID++;
    newNode->next = NULL;
  }
  else
  {
    free(newNode);
    newNode = NULL;
  }
  checkNode(newNode);
  return newNode;

} // end of makeNode()

//------------------------------------------------------
// destroyNode
//
// PURPOSE: destroys a node instance and its contents 
//
// INPUT PARAMETERS:
// A pointer to the node being destroyed
//------------------------------------------------------
static void destroyNode(Node* aNode)
{
  // destroy if node is valid
  checkNode(aNode);
  free(aNode);

} // end of destroyNode()

//------------------------------------------------------
// checkNode
//
// PURPOSE: invariant for the node struct. checks if a
//          node is valid.
//
// INPUT PARAMETERS:
// A pointer to the node being checked
//------------------------------------------------------
static void checkNode(Node* aNode)
{
  assert(aNode != NULL);
  assert(aNode->memStartIndex >= 0);
  assert(aNode->memSize > 0);
  assert(aNode->memSize <= MEMORY_SIZE);
  assert(aNode->objReferenceCount >= 0);
  assert(aNode->objReferenceID > 0);
  assert(aNode->objReferenceID < referenceID);

} //end of checkNode()

//------------------------------------------------------
// makeIndex
//
// PURPOSE: creates a new instance of the index struct
//
// RETURN:
// A pointer to the new index created
//------------------------------------------------------
static Index* makeIndex()
{
  Index* newIndex = (Index*)(malloc(sizeof(Index)));
  assert(newIndex != NULL);
  if(newIndex != NULL)
  {
    // index is empty when created
    newIndex->top = NULL;
  }
  else
  {
    free(newIndex);
    newIndex = NULL;
  }
  checkIndex(newIndex);
  return newIndex;

} //end of makeIndex()

//------------------------------------------------------
// destroyIndex
//
// PURPOSE: destroys any memory/resources being used by the
//          index instance and its contents 
//
// INPUT PARAMETERS:
// A pointer to the index being destroyed
//------------------------------------------------------
void destroyIndex(Index* anIndex)
{
  checkIndex(anIndex);

  // destroying each individual node in the linked list
  Node* curr = anIndex->top;
  Node* prev = NULL;
  while(curr != NULL)
  {
    prev = curr;
    curr = curr->next;
    destroyNode(prev);
  }
  free(anIndex);

} // end of destroyIndex()

//------------------------------------------------------
// checkIndex
//
// PURPOSE: invariant for the index struct. checks if an
//          index is valid.
//
// INPUT PARAMETERS:
// A pointer to the index being checked
//------------------------------------------------------
static void checkIndex(Index* anIndex)
{
  assert(anIndex != NULL);

  if(anIndex->top != NULL)
  {
    //checking each individual node in the linked list is also valid
    Node* curr = anIndex->top;
    while(curr != NULL)
    {
      checkNode(curr);
      curr = curr->next;
    }
  }
} // end of checkIndex()