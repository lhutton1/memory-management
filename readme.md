# Memory-management.c
Memory-management provides an implementation of the malloc and free functions. This implementation allows you to create a block on the heap and free it from memory, whilst ensuring the blocks are aligned correctly to inline with the x86 architecture requirements.

# How to use
Make sure that you are on a linux machine that is capable of using the sbrk() system call. Ensure that the correct padding for your machine is set by changing the `ALIGNMENT` and `PAGE_SIZE` constants. Then compile using a standard c compiler e.g:
`gcc memory-management.c -o memory-management`
