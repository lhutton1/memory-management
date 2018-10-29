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

    // no suitable block found, allocating more memory
    } else {
      block_meta *newBlock = create_block(size);
      searchBlock->next = newBlock;
    }

  // create initial block
  } else {
    block_meta *newBlock = create_block(size);
    block_head = newBlock;
  }
}

void _free(void *ptr)
{

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
  block_meta *newBlock = sbrk(newBlockSize + sizeof(block_meta));
  newBlock->size = newBlockSize;
  newBlock->free = false;
  newBlock->next = NULL;
  return newBlock;
}

block_meta *split_block(block_meta *blockToSplit, size_t requestedSize)
{
  block_meta *spareBlock = (block_meta *)((void *)blockToSplit + requestedSize + sizeof(block_meta));

  spareBlock->size = blockToSplit->size - requestedSize - sizeof(block_meta);
  spareBlock->free = true;
  spareBlock->next = blockToSplit->next;

  blockToSplit->size = requestedSize;
  blockToSplit->free = false;
  blockToSplit->next = spareBlock;

  return NULL;
}


// DEBUGGING
void print_block(block_meta *block) {
  if (block) {
    printf("Block: %p\n", block);
    printf("Block Size: %d\n", block->size);
    printf("Block Free: %d\n", block->free);
    printf("Block Next: %p\n", block->next);
  }
}

void print_blocks() {
  block_meta *currentBlock = block_head;

  while (currentBlock != NULL) {
    // check for a suitable block
    print_block(currentBlock);
    currentBlock = currentBlock->next;
    printf("---------\n");
  }
}

int main(void)
{
  printf("calling malloc 1..\n");
  void *call1 = _malloc(4096);

  printf("calling malloc 2..\n");
  void *call2 = _malloc(1024);

  printf("calling malloc 3..\n");
  void *call3 = _malloc(8192);

  printf("calling malloc 4..\n");
  void *call4 = _malloc(1024);

  //*((int*)call1) = 5;
  //printf("%d\n", *((int*)call1));

  print_blocks();
}
