//-----------------------------------------
//
// REMARKS: An automated test suite to adequately test
//          the object manager functionality. We will
//          test all functions in the provided interface
//          with general case and edge case test data.
//-----------------------------------------

#include "ObjectManager.h"
#include <stdio.h>
#include <stdlib.h>

// to keep track of total tests
int testsPassed = 0;
int testsFailed = 0;

//function prototypes
static void testInitDestPool();
static void testInsertObject();
static void testRetrieveObject();
static void testAddReference();
static void testDropReference();

/*
This function tests the functions from Object Manager
interface that initialise and destroy the memory pool.
*/
static void testInitDestPool()
{
  printf("\nTESTING INITIALISE MEMORY POOL and DESTROY MEMORY POOL\n\n");
  //Case 1: checking memory pool was actually initialised
  initPool();
  Ref testingRef = insertObject(100);

  if(testingRef != NULL_REF)
  {
    printf("1. SUCCESS: expected for memory to be allocated because memory pool was successfully initialised. Observed expected Behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: expected for memory to be allocated because memory pool was successfully initialised. Did not observe expected Behavior!.\n");
    testsFailed++;
  }

  //Case 2: checking memory pool was actually destroyed
  destroyPool();
  void* pointer = retrieveObject(testingRef);

  if(pointer == NULL)
  {
    printf("2. SUCCESS: memory pool was destroyed. Cannot access previously reserved memory anymore. Observed expected Behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: memory pool was destroyed. Cannot access previously reserved memory anymore. Did not observe expected Behavior!.\n");
    testsFailed++;
  }
  printf("----------------------------------------END OF TESTING initPool and destroyPool FUNCTION---------------------------------------\n");
}

/*
This function tests the function from Object Manager
interface that reserves memory in the memory pool.
*/
static void testInsertObject()
{
  printf("\nTESTING INSERT OBJECT FUNCTION\n\n");
  printf("---------------------------------------------Testing General Cases----------------------------------------------\n");
  initPool();
  // General Case 1: requesting 1000 bytes
  Ref testRef1 = insertObject(1000);

  if(testRef1 != NULL_REF)
  {
    printf("1. SUCCESS: expected for 1000 bytes to be reserved, and 1000 bytes were reserved.\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: expected for 1000 bytes to be reserved, and 1000 bytes were not reserved.\n");
    testsFailed++;
  }

  // General Case 2: requesting 500000 bytes
  Ref testRef2 = insertObject(500000);

  if(testRef2 != NULL_REF)
  {
    printf("2. SUCCESS: expected for 500,000 bytes to be reserved, and 500,000 bytes were reserved.\n");
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: expected for 500,000 bytes to be reserved, and 500,000 bytes were not reserved.\n");
    testsFailed++;
  }

  // General Case 3: requesting another 24000 bytes
  Ref testRef3 = insertObject(24000);

  if(testRef3 == NULL_REF)
  {
    printf("3. SUCCESS: expected for 24,000 bytes to not be reserved, and 24,000 bytes were not reserved.\n");
    testsPassed++;
  }
  else
  {
    printf("3. FAILED: expected for 24,000 bytes to not be reserved, and 24,000 bytes were reserved.\n");
    testsFailed++;
  }

  // General Case 4: requesting 24000 bytes again but initiating garbage collection this time to make room
  dropReference(testRef2);
  testRef3 = insertObject(24000);

  if(testRef3 != NULL_REF)
  {
    printf("4. SUCCESS: expected for garbage collection to activate and 24,000 bytes to be reserved, which is what happened.\n");
    testsPassed++;
  }
  else
  {
    printf("4. FAILED: expected for garbage collection to activate and 24,000 bytes to be reserved. This did not happen.\n");
    testsFailed++;
  }
  destroyPool();

  printf("\n-----------------------------------------------Testing Edge Cases-----------------------------------------------\n");

  initPool();
  // Edge Case 1: Attempting to reserve 0 bytes
  Ref testRef4 = insertObject(0);

  if(testRef4 == NULL_REF)
  {
    printf("1. SUCCESS: expected for the program to understand the redundancy of the request and return NULL_REF, which is what happened.\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: expected for the program to understand the redundancy of the request and return NULL_REF. This did not happen.\n");
    testsFailed++;
  }

  // Edge Case 2: Requesting more memory than what's available
  Ref testRef5 = insertObject(600000);

  if(testRef5 == NULL_REF)
  {
    printf("2. SUCCESS: cannot reserve more memory than what is available.\n");   
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: reserved more memory than what is available.\n");
    testsFailed++;
  }

  // Edge Case 3: attempting to reserve negative bytes
  Ref testRef = insertObject(-10);

  if(testRef == NULL_REF)
  {
    printf("3. SUCCESS: cannot reserve negative bytes. Expected to get NULL_REF and NULL_REF was returned.\n");
    testsPassed++;
  }
  else
  {
    printf("3. FAILED: cannot reserve negative bytes. Expected to get NULL_REF and NULL_REF was not returned.\n");
    testsFailed++;
  }

  destroyPool();

  // Edge Case 4: calling insertObject on an uninitialised object manager
  Ref testReference = insertObject(10);

  if(testReference == NULL_REF)
  {
    printf("4. SUCCESS: cannot reserve memory with no object manager initialised. Observed expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("4. FAILED: cannot reserve memory with no object manager initialised. Did not observe this expected behavior!.\n");
    testsFailed++;
  }

  printf("\n----------------------------------------END OF TESTING insertObject FUNCTION---------------------------------------\n");
}

/*
This function tests the function from Object Manager
interface that retrieves a pointer to reserved memory
in the memory pool.
*/
static void testRetrieveObject()
{
  printf("\nTESTING RETRIEVE OBJECT FUNCTION\n\n");
  printf("---------------------------------------------Testing General Cases----------------------------------------------\n");

  initPool();
  //General Case 1: Retreiving an object with a reference that exists
  Ref testRef6 = insertObject(20);
  void* ptr1 = retrieveObject(testRef6);

  if(ptr1 != NULL)
  {
    printf("1. SUCCESS: expected to retreive a pointer to location where memory was reserved, which is what happened.\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: expected to retreive a pointer to location where memory was reserved. This did not happen.\n");
    testsFailed++;
  }

  //General Case 2: Retreiving an object with a reference that exists after garbage collection initiated
  // create a new object, drop reference to an old object, insert another large object to fire GC
  Ref testRef7 = insertObject(2000);
  dropReference(testRef6);
  Ref testRef8 = insertObject(522286);
  void* ptr2 = retrieveObject(testRef7);

  if(ptr2 != NULL)
  {
    printf("2. SUCCESS: expected to retrieve a pointer to location where reserved memory was moved to after garbage collection initiated, which is what happened.\n");
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: expected to retrieve a pointer to location where reserved memory was moved to after garbage collection initiated. This did not happen.\n");
    testsFailed++;
  }

  printf("\n-----------------------------------------------Testing Edge Cases-----------------------------------------------\n");


  // Edge Case 1: Retrieving using a reference that doesn't exist in the memory pool
  Ref testRef9 = 98;
  void* ptr3 = retrieveObject(testRef9);

  if(ptr3 == NULL)
  {
    printf("1. SUCCESS: expected to not find anything i.e. NULL since ref doesn't exist, which is what happened.\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: expected to not find anything i.e. NULL since ref doesn't exist. This did not happen.\n");
    testsFailed++;
  }

  // Edge Case 2: Retrieving using a reference that existed previously but now the ref count has dropped to 0
  dropReference(testRef8);
  void* ptr4 = retrieveObject(testRef8);

  if(ptr4 == NULL)
  {
    printf("2. SUCCESS: cannot retrieve an object that has a reference count of 0.\n");
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: retrieved an object that has a reference count of 0.\n");
    testsFailed++;
  }

  // Edge Case 3: Using NULL_REF
  Ref testRef10 = insertObject(0);
  void* ptr5 = retrieveObject(testRef10);

  if(ptr5 == NULL)
  {
    printf("3. SUCCESS: cannot retrieve a pointer to memory that doesn't exist. Expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("3. FAILED: retrieved a pointer to memory that doesn't exist. Unexpected Behavior.\n");
    testsFailed++;
  }

  destroyPool();

  // Edge Case 4: calling retrieveObject on an uninitialised object manager
  void* ptr = retrieveObject(1);

  if(ptr == NULL)
  {
    printf("4. SUCCESS: cannot access uninitialised memory. Observed expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("4. FAILED: cannot access uninitialised memory. Did not observe this expected behavior!.\n");
    testsFailed++;
  }

  printf("\n----------------------------------------END OF TESTING retrieveObject FUNCTION---------------------------------------\n");
}

/*
This function tests the function from Object Manager
interface that adds another reference to an existing
object in the memory pool.
*/
static void testAddReference()
{
  printf("\nTESTING ADD REFERENCE FUNCTION\n\n");
  printf("---------------------------------------------Testing General Cases----------------------------------------------\n");
  initPool();
  // General Case 1: repeated calls to add reference. confirming each call adds one reference
  Ref testRef11 = insertObject(10200);
  // number of references to object currently
  int numReferences = 1;
  // adding 4 more refs
  while ( numReferences < 5)
  {
    addReference(testRef11);
    numReferences++;
  }

  // confirming there are a total of 5 references to the object
  int count = 0;
  while(retrieveObject(testRef11) != NULL)
  {
    dropReference(testRef11);
    count++;
  }

  if(numReferences == count)
  {
    printf("1. SUCCESS: each call adds one reference, confirmed by number of calls needed to drop reference for an object to go out of scope. Observed Expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: each call adds one reference, confirmed by number of calls needed to drop reference for an object to go out of scope.. Did not observe expected Behavior.\n");
    testsFailed++;
  }
  destroyPool();

  //General Case 2: Adding reference to an object that exists in the pool
  //shown by call to insert as garbage collection won't compact anything
  initPool();
  Ref testRef12 = insertObject(500000);
  addReference(testRef12);
  Ref testRef13 = insertObject(25000);

  // no garbage thus no room created
  if(testRef13 == NULL_REF)
  {
    printf("2. SUCCESS: Object did not go out of scope thus no garbage should be collected and no new memory should be allocated consequently, which is what happened.\n");
    dumpPool();
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: Object did not go out of scope thus no garbage should be collected and no new memory should be allocated consequently. This did not happen.\n");
    testsFailed++;
  }

  destroyPool();
  printf("\n-----------------------------------------------Testing Edge Cases-----------------------------------------------\n");

  initPool();

  // Edge Case 1: adding reference to an object that doesn't exist in the memory pool i.e. ref id was not handed out
  Ref testRef14 = 1000;
  addReference(testRef14);
  void* ptr6 = retrieveObject(testRef14);
  // if the object existed, we would be able to retrieve it since we added a reference to it
  if(ptr6 == NULL)
  {
    printf("1. SUCCESS: expected to not find anything since there are no references to an object wit ref id 1000, which is what happened.\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: expected to not find anything since there are no references to an object wit ref id 1000. This did not happen.\n");
    testsFailed++;
  }

  // Edge Case 2: adding reference to an object that existed previously but now the ref count has dropped to 0
  Ref testRef15 = insertObject(34000);
  dropReference(testRef15);
  addReference(testRef15);
  void* ptr7 = retrieveObject(testRef15);

  if(ptr7 == NULL)
  {
    printf("2. SUCCESS: cannot add references to an object that went out of scope. Observed expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: cannot add references to an object that went out of scope. Did not observe expected behavior!\n");
    testsFailed++;
  }

  // Edge Case 3: Using NULL_REF with addReference()
  Ref testRef16 = insertObject(1024*550); // inserting more than available thus no allocation happens
  addReference(testRef16);
  void* ptr8 = retrieveObject(testRef16);

  if(ptr8 == NULL)
  {
    printf("3. SUCCESS: cannot add a reference to an object that wasn't allocated memory in the pool to begin with. Observed expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("3. FAILED: cannot add a reference to an object that wasn't allocated memory in the pool to begin with. Did not observe expected Behavior.\n");
    testsFailed++;
  }

  destroyPool();
  printf("\n----------------------------------------END OF TESTING addReference FUNCTION---------------------------------------\n");
}

/*
This function tests the function from Object Manager
interface that decrements the number of references to
an object in the memory pool.
*/
static void testDropReference()
{
  printf("\nTESTING DROP REFERENCE FUNCTION\n\n");
  printf("---------------------------------------------Testing General Cases----------------------------------------------\n");

  initPool();
  // General Case 1: repeated calls to drop reference. confirming each call drops one reference
  // number of calls to drop ref = number of calls to add ref + 1 (for an object to go out of scope)
  Ref testRef17 = insertObject(23400);
  int callsAddRef = 0;
  while ( callsAddRef < 5)
  {
    addReference(testRef17);
    callsAddRef++;
  }

  int callsDropRef = 0;

  while(retrieveObject(testRef17) != NULL)
  {
    dropReference(testRef17);
    callsDropRef++;
  }

  if(callsDropRef == (callsAddRef + 1))
  {
    printf("1. SUCCESS: number of calls needed to 'dropReference' must equal number of calls to 'addreference + 1' for an object to go out of scope. Observed Expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: number of calls needed to 'dropReference' must equal number of calls to 'addreference + 1' for an object to go out of scope. Did not observe expected Behavior.\n");
    testsFailed++;
  }
  destroyPool();

  //General Case 2: Dropping references to objects in the memory pool to test impact on garbage collection initiation
  initPool();

  Ref testRef18 = insertObject(30040);
  Ref testRef19 = insertObject(150000);
  Ref testRef20 = insertObject(250000);
  Ref testRef21 = insertObject(94000);
  //adding some references and dropping some references
  addReference(testRef18);
  dropReference(testRef21);
  dropReference(testRef20);
  addReference(testRef19);

  Ref testRef22 = insertObject(1050);
  // garbage collection fired and room created for new object as drop reference made some objects go out of scope
  if(testRef22 !=  NULL_REF)
  {
    printf("2. SUCCESS: expected for garbage collection to make room for the new object by cleaning up objects for which we dropped references, which is what happened.\n");
    dumpPool();
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: expected for garbage collection to make room for the new object by cleaning up objects for which we dropped references. This did not happen.\n");
    testsFailed++;
  }

  destroyPool();
  printf("\n-----------------------------------------------Testing Edge Cases-----------------------------------------------\n");

  initPool();

  // Edge Case 1: dropping reference for an object that doesn't exist in the memory pool i.e. ref id was not handed out
  Ref testRef23 = 23;
  dropReference(testRef23);
  Ref testRef24 = insertObject(MEMORY_SIZE);

  if(testRef24 != NULL_REF)
  {
    printf("1. SUCCESS: expected for memory to be allocated for the new object without GC running since no garbage was available and no space needed to be created, which is what happened.\n");
    testsPassed++;
  }
  else
  {
    printf("1. FAILED: expected for memory to be allocated for the new object without GC running since no garbage was available and no space needed to be created. This did not happen.\n");
    testsFailed++;
  }

  // Edge Case 2: dropping reference for an object that is already out of scope
  dropReference(testRef24);
  dropReference(testRef24);
  void* ptr9 = retrieveObject(testRef24);

  if(ptr9 == NULL)
  {
    printf("2. SUCCESS: dropping references for an object that is already out of scope has no effect and object stays garbage/out of scope. Observed expected behavior!\n");
    testsPassed++;
  }
  else
  {
    printf("2. FAILED: dropping references for an object that is already out of scope has no effect and object stays garbage/out of scop. Did not observe expected behavior!\n");
    testsFailed++;
  }

  destroyPool();

  printf("\n----------------------------------------END OF TESTING dropReference FUNCTION---------------------------------------\n");
}

int main()
{
  //calling all test functions
  testInitDestPool();
  testInsertObject();
  testRetrieveObject();
  testAddReference();
  testDropReference();

  //final Summary
  printf("\n---------------------------------------------FINAL TESTING SUMMARY----------------------------------------------\n");
  printf("Total number of tests executed: %d\n", (testsPassed + testsFailed));
  printf("Number of tests passed: %d\n", testsPassed);
  printf("Number of tests failed: %d\n", testsFailed);

  printf("\nEND OF PROGRAM\n");
  return 0;
}