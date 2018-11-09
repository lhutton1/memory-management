////////////////////////////////// Start Header ////////////////////////////////
#include "memory-management.h"

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
const int PAGE_SIZE = 4096;

// helper functions
block_meta *search_blocks(size_t newBlockSize);
block_meta *create_block(size_t newBlockSize);
void split_block(block_meta *blockToSplit, size_t requestedSize);
block_meta *merge();

/////////////////////////////////// End Header /////////////////////////////////

/*
 * Malloc allocates a new block of memory on the heap.
 *
 * Size - the requested size of the new block of memory.
 * Returns a void * pointer to the start of the allocated memory.
 */
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
  } else if (searchBlock->free && searchBlock->size >= size + sizeof(block_meta)) {
      split_block(searchBlock, size);
      return searchBlock + sizeof(block_meta);

    // no suitable block found, allocating more memory
    } else {
      block_meta *newBlock = create_block(size);

      if (!newBlock)
        return NULL;

      searchBlock->next = newBlock;
      newBlock->prev = searchBlock;
      return newBlock + sizeof(block_meta);
    }

  // create initial block
  } else {
    block_meta *newBlock = create_block(size);

    if (!newBlock)
      return NULL;

    block_head = newBlock;
    return newBlock + sizeof(block_meta);
  }
}

/*
 * Free a block of memory from the heap. This returns memory bacl to the heap
 * and occasionally back to the operating system.
 *
 * Ptr - the start of the memory address that is to be freed.
 */
void _free(void *ptr)
{
  if (!ptr)
    return;

  // free the requested block
  block_meta *blockToFree = (block_meta *)ptr - sizeof(block_meta);

  if (blockToFree->free)
    return;

  blockToFree->free = true;

  // merge any free blocks that are next to each other
  block_meta *lastBlock = merge();

  // now we need to deallocate the end block if it is possible.
  // This reduces the size of the heap.
  if (lastBlock->free) {
    int deallocateMultiple = (lastBlock->size + sizeof(block_meta)) / PAGE_SIZE;

    if (lastBlock->prev && deallocateMultiple >= 1) {
      lastBlock->prev->next = NULL;
      lastBlock->size -= deallocateMultiple * PAGE_SIZE;
      sbrk(-(deallocateMultiple * PAGE_SIZE));

    // if this is the last block and all other blocks have been freed
    // deallocate remaining memory and set the block_head to NULL.
    } else if (!lastBlock->prev){
      block_head = NULL;
      sbrk(-PAGE_SIZE);
    }
  }
}


/////////////////////////////// Helper Functions ///////////////////////////////
/*
 * Search the blocks consectively checking to see if a suitable free block
 * can be used for the requested size. If there is return it, if not return
 * the last block in the linked list.
 *
 * newBlockSize - the requested size of the new block
 * return block_meta*
 */
block_meta *search_blocks(size_t newBlockSize)
{
  block_meta *currentBlock = block_head;

  while (true) {
    // check for a suitable block
    if (currentBlock->free && currentBlock->size >= newBlockSize + sizeof(block_meta))
      return currentBlock;

    // check if end of list reached
    if (currentBlock->next == NULL)
      return currentBlock;
    else
      currentBlock = currentBlock->next;
  }
}


/*
 * Create a new block in the heap by calling sbrk to allocate more memory.
 * Returns the block that has been created or NULL if there was an issue.
 *
 * newBlockSize - the requested size of the new block
 * return block_meta*
 */
block_meta *create_block(size_t newBlockSize)
{
  block_meta *newBlock;
  int allocateSize;

  // the aim here is to reduce the number of calls to sbrk. So when we call
  // sbrk we only call it in multiples of the 'PAGE_SIZE' constant
  const int totalSize = newBlockSize + sizeof(block_meta);
  const int rem = totalSize % PAGE_SIZE;

  if (rem == 0)
    allocateSize = totalSize;
  else
    allocateSize = totalSize + PAGE_SIZE - rem;

  // create the new block of memory and split it down to the correct size
  // that was initially requested for.
  newBlock = (block_meta *)sbrk(allocateSize);

  if (!newBlock)
    return NULL;

  newBlock->size = allocateSize - sizeof(block_meta);
  newBlock->free = false;
  newBlock->next = NULL;
  newBlock->prev = NULL;

  if (newBlockSize < allocateSize)
    split_block(newBlock, newBlockSize);

  return newBlock;
}


/*
 * Split a block down to the size of the requested amount. Leaving another free
 * block with the remaining size or the original block.
 *
 * blockToSplit - pointer to the block that needs to be split
 * requestedSize - the requested size of the new block
 */
void split_block(block_meta *blockToSplit, size_t requestedSize)
{
  block_meta *spareBlock = (block_meta *)((void *)blockToSplit + sizeof(block_meta) + requestedSize);

  // This process may result in the spare block not being aligned properly.
  // To align properly we need to add padding.
  int padding = ALIGNMENT - ((uintptr_t)spareBlock % ALIGNMENT);
  spareBlock = (block_meta *)((void *)spareBlock + padding);

  if (padding >= ALIGNMENT)
    padding = 0;

  // now we initialise both the block to split and the spare block
  spareBlock->size = blockToSplit->size - requestedSize - sizeof(block_meta) - padding;
  spareBlock->free = true;
  spareBlock->next = blockToSplit->next;
  spareBlock->prev = blockToSplit;

  blockToSplit->size = requestedSize;
  blockToSplit->free = false;
  blockToSplit->next = spareBlock;
}


/*
 * Merge any consecutive free blocks after a block has been freed,
 * reducing the number of blocks in the linked list.
 *
 * return block_meta* - the last block in the list.
 */
block_meta *merge()
{
  block_meta *currentBlock = block_head;

  // iterate over blocks checking if there are 2 consecutive free blocks.
  // If 2 are found then they are merged and the loop pauses to check
  // if any more blocks need merging before continuing.
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
