#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

// Datatype that stores data about the block.
// Size - size of the block (not including the metadata)
// Free - has the block been freed by the process?
// Prev - the previous block in the linked list
// Next - the next block in the linked list
typedef struct block_meta {
  size_t size;
  bool free;
  struct block_meta *prev;
  struct block_meta *next;
} block_meta;

// size of the metadata used in each block.
const int META_SIZE = 16;

// head of doubly linked list block.
block_meta *block_head = NULL;

// Allocates new block of memory.
void *_malloc(size_t size);

// Frees a block of memory.
void _free(void *ptr);

#endif
