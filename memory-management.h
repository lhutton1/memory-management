#ifndef __MEMORY_MANAGEMENT__
#define __MEMORY_MANAGEMENT__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// Allocates new block of memory.
void *_malloc(size_t size);

// Frees a block of memory.
void _free(void *ptr);

#endif
