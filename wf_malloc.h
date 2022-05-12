//
// Created on 5/9/22.
//

/*
 * This is an emulation for the worst fit algorithm in memory allocation. It does allocate the memory via mmap,
 * but never unmap the memory. It implements the algorithm by maintaining a single linked list. For each malloc,
 * it searches the list to find the largest unused chunk, return the address as it's allocated. If even the
 * largest unused chunk is smaller than the requested size, then it will really allocate new memory via mmap.
 */

/*
 * This malloc.h uses mmap instead of sbrk since the function sbrk() is withdrawn by Single Unix Standard, Version 3
 * and is not compatible with any other memory allocation method. The sbrk() just simply resize the data segment
 * to gain a larger space, resulting a lot of problem hard to track and solve. The alternative and the recommended one
 * is mmap by all up-to-date document, such as the IBM's.
 */

#ifndef WF_MALLOC_WF_MALLOC_H
#define WF_MALLOC_WF_MALLOC_H

// to use errno
#include <errno.h>
// to use NULL macro
#include <stddef.h>
// to use memset()
#include <string.h>
// to use mmap()
#include <sys/mman.h>
/* to get page size. But this example processes the memory into small block sized by union chunk, and requires
 * contiguous address, so it will not map at exactly address suitably aligned by page size.
 */
#include <unistd.h>

/*
 * This is an example to emulate the worst fit algorithm of memory allocation.
 * So it should have the least size at each new memory allocation.
 * Otherwise, there would be no enough space for newly requests always.
 * In this case, I set it to be 1024 blocks, which equals to 16384 bytes.
 */

union chunk {
    struct {
        /* This field is the next node pointer obviously */
        union chunk *next;
        /*
         * This field store the available block number of the chunk. A block's size equals to
         * the size of the union chunk, 16 bytes, or a double-word in amd64 or aarch64 environment.
         * This memory management emulator allocate least a block.
         */
        size_t block_num;
    } meta;

    /*
     * The alignment does no sense, since the struct meta's size is 16 bytes, having reached a double-word
     * under amd64 and aarch64 environment. And on 2022, popular GNU/Linux distributions only provides 64-bit
     * images.
     */
    //long x;
};

typedef union chunk node;

// A convenient macro to jump over the chunk head
#define chunk2mem(p) (void *)((char *)(p) + sizeof(union chunk))

// A convenient macro to calculate the required block number
#define block_count(s) (((s) + sizeof(union chunk) - 1) / sizeof(union chunk))

// The header of the linked list. It will be initialized at first memory allocate
static node *list = NULL;

// emulation of malloc function
void *wf_malloc(size_t size);

// emulation of calloc function
void *wf_calloc(size_t num, size_t size);

// emulation of free function
void wf_free(void *ptr);

#endif //WF_MALLOC_WF_MALLOC_H
