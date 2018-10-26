#include "memory-management.h"

void *_malloc(size_t size)
{
  block_meta *newBlock = NULL;

  // nothing to malloc
  if (size == 0) return NULL;

  // search to find suitable block
  if (block_head) newBlock = search_blocks(size);

  if (newBlock) {
    split_block(newBlock);
  } else {
    // create a new block if no suitable block were found
    printf("creating new block..\n");
    newBlock = create_block(size);
    if (block_head) block_head->next = newBlock;
    block_head = newBlock;
  }

  return newBlock;
}

void _free(void *ptr)
{

}

// helper functions
block_meta *search_blocks(size_t newBlockSize)
{
  block_meta *currentBlock = block_head;

  while (currentBlock != NULL) {
    // check for a suitable block
    if (currentBlock->free && currentBlock->size >= newBlockSize) {
      return currentBlock;
    }
    currentBlock = currentBlock->prev;
  }
  return NULL;
}

block_meta *create_block(size_t newBlockSize)
{
  block_meta *newBlock = sbrk(newBlockSize + META_SIZE);
  newBlock->size = newBlockSize;
  newBlock->free = true;
  newBlock->prev = block_head;
  newBlock->next = NULL;
  return newBlock;
}

block_meta *split_block(block_meta *blockToSplit, size_t requestedSize)
{
  // addr 1.........
  // addr 1 | addr 2
  // meta - addr 1 | meta - addr 2
  //block_meta *spareBlock;

  //spareBlock->size = blockToSplit->size - requestedSize;

  //blockToSplit->size = ;
  //blockToSplit->next = ;
  //blockToSplit->free = ;

}

void print_block(block_meta *block) {
  if (block) {
    printf("Block: %p\n", block);
    printf("Block Size: %d\n", block->size);
    printf("Block Free: %d\n", block->free);
    printf("Block Prev: %p\n", block->prev);
    printf("Block Next: %p\n", block->next);
  }
}

int main(void)
{
  printf("calling malloc 1..\n");
  block_meta *call1 = _malloc(2048);

  printf("calling malloc 2..\n");
  block_meta *call2 = _malloc(1024);

  //printf("calling malloc 3..\n");
  //block_meta *call3 = _malloc(1024);

  //printf("calling malloc 4..\n");
  //block_meta *call4 = _malloc(1024);

  printf("---------\n");
  print_block(call1);
  printf("---------\n");
  print_block(call2);
  printf("---------\n");
  //print_block(call3);
  //printf("---------\n");
  //print_block(call4);
  //printf("---------\n");
}
