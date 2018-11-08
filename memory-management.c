////////////////////////////////// Start Header ////////////////////////////////
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// Datatype that stores data about the block.
// Size - size of the block (not including the metadata)
// Free - has the block been freed by the process?
// Prev - the previous block in the linked list
// Next - the next block in the linked list
typedef struct block_meta {
  size_t size;
  bool free;
  struct block_meta *next;
  struct block_meta *prev;
} block_meta;

// head of doubly linked list block.
block_meta *block_head = NULL;

// alignment
const int ALIGNMENT = 8;
const int NEW_BLOCK_MULTIPLE = 4096;

// Allocates new block of memory.
void *_malloc(size_t size);

// Frees a block of memory.
void _free(void *ptr);

block_meta *search_blocks(size_t newBlockSize);
block_meta *create_block(size_t newBlockSize);
void split_block(block_meta *blockToSplit, size_t requestedSize);
block_meta *merge();

void print_block(block_meta *block);
void print_blocks();

/////////////////////////////////// End Header /////////////////////////////////


void *_malloc(size_t size)
{
  // nothing to malloc
  if (size == 0) return NULL;

  // search to find suitable block
  if (block_head) {
    block_meta *searchBlock = NULL;
    searchBlock = search_blocks(size);

    // free block found of exactly the same size
    if (searchBlock->free && searchBlock->size == size) {
      searchBlock->free = false;
      return searchBlock + sizeof(block_meta);

    // free block found of size greater than requested
    } else if (searchBlock->free && searchBlock->size >= size) {
      split_block(searchBlock, size);
      return searchBlock + sizeof(block_meta);

    // no suitable block found, allocating more memory
    } else {
      block_meta *newBlock = create_block(size);
      searchBlock->next = newBlock;
      newBlock->prev = searchBlock;
      return newBlock + sizeof(block_meta);
    }

  // create initial block
  } else {
    block_meta *newBlock = create_block(size);
    block_head = newBlock;
    return newBlock + sizeof(block_meta);
  }
}


void _free(void *ptr)
{
  block_meta *blockToFree = (block_meta *)ptr - sizeof(block_meta);
  blockToFree->free = true;

  // merge any free blocks that are next to each other
  block_meta *lastBlock = merge();

  // now we need to deallocate the end block if it is possible.
  // This reduces the size of the heap.
  if (lastBlock->free) {
    int deallocateMultiple = (lastBlock->size + sizeof(block_meta)) / NEW_BLOCK_MULTIPLE;

    if (lastBlock->prev && deallocateMultiple >= 1) {
      printf("removing block\n");
      lastBlock->prev->next = NULL;
      lastBlock->size -= deallocateMultiple * NEW_BLOCK_MULTIPLE;
      sbrk(-(deallocateMultiple * NEW_BLOCK_MULTIPLE));

    // if this is the last block and all other blocks have been freed
    // deallocate remaining memory and set the block_head to NULL.
    } else if (!lastBlock->prev){
      block_head = NULL;
      sbrk(-/*NEW_BLOCK_MULTIPLE*/(lastBlock->size + sizeof(block_meta)));
    }
  }
}


/////////////////////////////// Helper Functions ///////////////////////////////
block_meta *search_blocks(size_t newBlockSize)
{
  block_meta *currentBlock = block_head;

  while (true) {
    // check for a suitable block
    if (currentBlock->free && currentBlock->size >= newBlockSize)
      return currentBlock;

    // check if end of list reached
    if (currentBlock->next == NULL)
      return currentBlock;
    else
      currentBlock = currentBlock->next;
  }
}


block_meta *create_block(size_t newBlockSize)
{
  void *newAddr;
  block_meta *newBlock;
  int allocateSize;

  // the aim here is to reduce the number of calls to sbrk. So when we call
  // sbrk we only call it in multiples of the 'NEW_BLOCK_MULTIPLE' constant
  const int totalSize = newBlockSize + sizeof(block_meta);
  const int rem = totalSize % NEW_BLOCK_MULTIPLE;

  if (rem == 0)
    allocateSize = totalSize;
  else
    allocateSize = totalSize + NEW_BLOCK_MULTIPLE - rem;

  // create the new block of memory and split it down to the correct size
  // that was initially requested for.
  newBlock = (block_meta *)sbrk(allocateSize);
  newBlock->size = allocateSize - sizeof(block_meta);
  newBlock->free = false;
  newBlock->next = NULL;
  newBlock->prev = NULL;

  if (newBlockSize < allocateSize)
    split_block(newBlock, newBlockSize);

  return newBlock;
}


void split_block(block_meta *blockToSplit, size_t requestedSize)
{
  block_meta *spareBlock = (block_meta *)((void *)blockToSplit + sizeof(block_meta) + requestedSize);

  // this process may result in the spare block not being aligned properly.
  // To align properly we need to add padding.
  int padding = ALIGNMENT - ((uintptr_t)spareBlock % ALIGNMENT);
  spareBlock = (block_meta *)((void *)spareBlock + padding);

  if (padding >= ALIGNMENT)
    padding = 0;

  printf("Padding added %d\n", padding);

  // now we initialise both the block to split and the spare block
  spareBlock->size = blockToSplit->size - requestedSize - sizeof(block_meta) - padding;
  spareBlock->free = true;
  spareBlock->next = blockToSplit->next;
  spareBlock->prev = blockToSplit;

  blockToSplit->size = requestedSize;
  blockToSplit->free = false;
  blockToSplit->next = spareBlock;
}


block_meta *merge()
{
  block_meta *currentBlock = block_head;

  // iterate over blocks checking if there are
  // 2 consecutive free blocks
  while(currentBlock->next != NULL) {
    if (currentBlock->free && currentBlock->next->free) {
      currentBlock->size += currentBlock->next->size;

      if (currentBlock->next->next)
        currentBlock->next = currentBlock->next->next;
      else {
        currentBlock->next = NULL;
      }
    } else {
      currentBlock = currentBlock->next;
    }
  }
  // return the last block
  return currentBlock;
}



// DEBUGGING
void print_block(block_meta *block)
{
  if (block) {
    printf("Block: %d\n", block);
    printf("Block Size: %d\n", block->size);
    printf("Block Free: %d\n", block->free);
    printf("Block Next: %d\n", block->next);
    printf("Block Prev: %d\n", block->prev);
    printf("Block alignment: %d\n", (uintptr_t)block % 8);
  }
}


void print_blocks()
{
  block_meta *currentBlock = block_head;
  printf("---------\n");

  while (currentBlock != NULL) {
    // check for a suitable block
    print_block(currentBlock);
    currentBlock = currentBlock->next;
    printf("---------\n");
  }
}



int main(void)
{
  void *call1 = _malloc(3);
  void *call2 = _malloc(4);
  void *call3 = _malloc(5768);
  void *call4 = _malloc(5768);
  void *call5 = _malloc(5768);
  void *call6 = _malloc(5768);
  void *call7 = _malloc(5768);
  void *call8 = _malloc(5768);
  print_blocks();
  _free(call1);
  _free(call4);
  _free(call6);
  _free(call3);
  _free(call2);
  _free(call5);
  _free(call8);
  _free(call7);

  void *call9 = _malloc(1);
  print_blocks();
  _free(call9);

  //void *call2 = _malloc(1024);
  //void *call3 = _malloc(8192);
  //void *call4 = _malloc(8192);
  //void *call5 = _malloc(100000);
  //_free(call1);
  //_free(call3);
  //_free(call2);
  //_free(call4);
  //void *call6 = _malloc(3);
  //void *call7 = _malloc(4);
  //_free(call5);
  //_free(call6);
  //print_blocks();
  //_free(call7);
  //void *call8 = _malloc(4096);
  //_free(call8);

}
