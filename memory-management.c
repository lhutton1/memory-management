#include "memory-management.h"

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
      return newBlock + sizeof(block_meta);
    }

  // create initial block
  } else {
    block_meta *newBlock = create_block(size);
    block_head = newBlock;
    return (void *)(newBlock + sizeof(block_meta));
  }
}


void _free(void *ptr)
{
  block_meta *blockToFree = (block_meta *)ptr - sizeof(block_meta);
  blockToFree->free = true;

  // merge any free blocks that are next to each other
  merge();
}


// helper functions
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
  const int rem = (totalSize) % NEW_BLOCK_MULTIPLE;

  if (rem == 0)
    allocateSize = totalSize;
  else
    allocateSize = totalSize + NEW_BLOCK_MULTIPLE - rem;

  // create the new block of memory and split it down to the correct size
  // that was initially requested for.
  newBlock = (block_meta *)sbrk(allocateSize);
  newBlock->size = allocateSize;
  newBlock->free = false;
  newBlock->next = NULL;

  if (newBlockSize < allocateSize)
    split_block(newBlock, newBlockSize);

  return newBlock;
}


block_meta *split_block(block_meta *blockToSplit, size_t requestedSize)
{
  block_meta *spareBlock = (block_meta *)((void *)blockToSplit + requestedSize + sizeof(block_meta));

  // this process may result in the spare block not being aligned properly.
  // To align properly we need to add padding.
  int padding = ALIGNMENT - ((uintptr_t)spareBlock % ALIGNMENT);
  spareBlock = (block_meta *)((void *)spareBlock + padding);

  // now we initialise both the block to split and the spare block
  spareBlock->size = blockToSplit->size - requestedSize - sizeof(block_meta);
  spareBlock->free = true;
  spareBlock->next = blockToSplit->next;

  blockToSplit->size = requestedSize;
  blockToSplit->free = false;
  blockToSplit->next = spareBlock;

  return NULL;
}


void merge()
{
  block_meta *currentBlock = block_head;

  // iterate over blocks checking if there are
  // 2 consecutive free blocks
  while(currentBlock->next != NULL) {
    if (currentBlock->free && currentBlock->next->free) {
      currentBlock->size += currentBlock->next->size;

      if (currentBlock->next->next)
        currentBlock->next = currentBlock->next->next;
      else
        currentBlock->next = NULL;
        return;
    }

    currentBlock = currentBlock->next;
  }
}



// DEBUGGING
void print_block(block_meta *block)
{
  if (block) {
    printf("Block: %p\n", block);
    printf("Block Size: %d\n", block->size);
    printf("Block Free: %d\n", block->free);
    printf("Block Next: %p\n", block->next);
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
  void *call1 = _malloc(4096);
  void *call2 = _malloc(1024);
  void *call3 = _malloc(8192);
  _free(call1);
  _free(call2);
  _free(call3);
  void *call4 = _malloc(1024);
  _free(call4);
  void *call5 = _malloc(1024);
  void *call6 = _malloc(5555);
  void *call7 = _malloc(5789);
  void *call8 = _malloc(4576);
  _free(call8);
  _free(call5);
  void *call9 = _malloc(5555);
  void *call10 = _malloc(5789);
  void *call11 = _malloc(4576);
  _free(call6);
  void *call12 = _malloc(55356);
  void *call13 = _malloc(1);
  void *call14 = _malloc(567);

  print_blocks();
}
