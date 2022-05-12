//
// Created on 5/9/22.
//

#include "wf_malloc.h"

void *wf_malloc(size_t size) {
    node *cur_ptr = list;
    // store the pointer to the largest block
    node *target_node = NULL;
    size_t largest_chunk = 0;

    // count the required block number which is rounded up.
    size_t required = block_count(size);
    if (size <= 0) {
        return NULL;
    }

    // Initialize the linked list at first.
    if (list == NULL) {
        list = (node *) mmap(NULL, sizeof(node),
                             PROT_READ | PROT_WRITE,
                             MAP_ANONYMOUS | MAP_PRIVATE,
                             0,
                             0);

        // attach a tail to the list.
        node *next = (node *) mmap(((char *) list + sizeof(union chunk)), getpagesize(),
                                   PROT_READ | PROT_WRITE,
                                   MAP_ANONYMOUS | MAP_PRIVATE,
                                   0,
                                   0);
        next->meta.next = NULL;
        next->meta.block_num = block_count(getpagesize()) - 1; // the chunk header occupied a block
        list->meta.next = next;
        list->meta.block_num = 0;
        cur_ptr = list;
    }

    // Iterate through the linked list to find the largest block
    while (cur_ptr->meta.next != NULL) {
        if (cur_ptr->meta.block_num > largest_chunk) {
            largest_chunk = cur_ptr->meta.block_num;
            target_node = cur_ptr;
        }
        cur_ptr = cur_ptr->meta.next;
    }
    // travel to the tail
    if (cur_ptr->meta.block_num > largest_chunk) {
        largest_chunk = cur_ptr->meta.block_num;
        target_node = cur_ptr;
    }

    // If current space cannot fit the required size, allocate new memory.
    // Mention that the cur_ptr is always point to the tail of the list.
    if (largest_chunk < required) {
        // we need the least size for each new memory allocation.
        size_t alloc_num = (required * sizeof(union chunk) + getpagesize() - 1) / getpagesize();

        // add a chunk that has enough space
        node *new_chunk = (node *) mmap((char *) cur_ptr + (cur_ptr->meta.block_num + 1) * sizeof(union chunk),
                                        getpagesize() * alloc_num, PROT_READ | PROT_WRITE,
                                        MAP_ANONYMOUS | MAP_PRIVATE,
                                        0,
                                        0);
        // the new chunk will be the node before tail.
        // In other words, always attach an unused node to the tail of the list
        new_chunk->meta.next = NULL;
        new_chunk->meta.block_num = block_count(getpagesize() * alloc_num) - 1;
        node *tail_chunk = (node *) mmap((char *) new_chunk + (new_chunk->meta.block_num + 1) * sizeof(union chunk),
                                         getpagesize(), PROT_READ | PROT_WRITE,
                                         MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        tail_chunk->meta.next = NULL;
        tail_chunk->meta.block_num = block_count(getpagesize()) - 1;
        // link the new chunk
        new_chunk->meta.next = tail_chunk;
        cur_ptr->meta.next = new_chunk;
        // this new chunk will be our wanted
        target_node = new_chunk;
        largest_chunk = new_chunk->meta.block_num;
    }

    if (largest_chunk == required) {
        // If the largest unallocated chunk size just equal to we need
        target_node->meta.block_num = 0;
        // The occupied chunk's block_num is set to 0, which determines a block is in use or not.
    } else {
        // If we found the largest unallocated chunk that just larger than the requested size
        node *node_to_insert = (node *) ((char *) target_node + sizeof(union chunk) * (required + 1));
        // insert an upper limit for the allocated memory
        node_to_insert->meta.next = target_node->meta.next;
        node_to_insert->meta.block_num = target_node->meta.block_num - (required + 1);
        target_node->meta.next = node_to_insert;
        target_node->meta.block_num = 0;
    }

    return chunk2mem(target_node);
}

// Just call wf_malloc and memset the value to 0
void *wf_calloc(size_t num, size_t size) {
    if (num <= 0 || size <= 0) {
        return NULL;
    }
    void *ptr = wf_malloc(num * size);
    memset(ptr, 0, block_count(num * size));

    return ptr;
}

void wf_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    // DO NOT use free before any alloc
    if (list == NULL) {
        return;
    }

    node *cur_ptr = list;
    node *prev = cur_ptr;

    // Iterate through the list
    while (cur_ptr->meta.next != NULL) {
        if (chunk2mem(cur_ptr) == ptr) {
            break;
        }
        prev = cur_ptr;
        cur_ptr = cur_ptr->meta.next;
    }
    // cannot find the allocated ptr, thus no way to deallocate the memory
    if (chunk2mem(cur_ptr) != ptr) {
        errno = EFAULT;
        return;
    }

    // Now we found the node

    // deallocate the current node
    node *next_block = cur_ptr->meta.next;
    size_t offset = (char *) (next_block) - (char *) (cur_ptr);
    cur_ptr->meta.block_num = offset / sizeof(union chunk) - 1;

    // merge the next node if possible
    if (next_block->meta.block_num > 0) {
        cur_ptr->meta.block_num += (next_block->meta.block_num + 1);
        cur_ptr->meta.next = next_block->meta.next;
    } else if ((next_block->meta.next) != NULL) {
        if (((char *) (next_block->meta.next) - (char *) next_block) == sizeof(union chunk)) {
            // the next node may be a node without any available space
            cur_ptr->meta.next = next_block->meta.next;
            cur_ptr->meta.block_num += 1;
        }
    }

    // be merged with the previous node if possible
    if (prev->meta.block_num > 0) {
        prev->meta.next = cur_ptr->meta.next;
        prev->meta.block_num += (cur_ptr->meta.block_num + 1);
    }
}