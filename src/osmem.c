#include <string.h>
#include <unistd.h>

#include "osmem.h"
#include "common.h"
#include "block.h"
#include "heap.h"

static void *alloc_with_threshold(size_t size, size_t threshold) {
    struct block_meta *block;

    size = ALIGN(size);

    if (META_SIZE + size >= threshold)
        block = mmap_alloc_block(size);
    else
        block = heap_alloc_block(size);

    return BLOCK_TO_PAYLOAD(block);
}

void *os_malloc(size_t size) {
    if (size == 0)
        return NULL;

    return alloc_with_threshold(size, MMAP_THRESHOLD);
}

void os_free(void *ptr) {
    struct block_meta *block;

    if (!ptr)
        return;

    block = PAYLOAD_TO_BLOCK(ptr);

    if (block->status == STATUS_MAPPED)
        mmap_free_block(block);
    else if (block->status == STATUS_ALLOC)
        block->status = STATUS_FREE;
}

void *os_calloc(size_t nmemb, size_t size) {
    size_t total;
    void *ptr;

    if (nmemb == 0 || size == 0)
        return NULL;

    if (nmemb > (size_t)-1 / size)
        return NULL;

    total = nmemb * size;
    ptr = alloc_with_threshold(total, (size_t)getpagesize());
    memset(ptr, 0, total);

    return ptr;
}

void *os_realloc(void *ptr, size_t size) {
    struct block_meta *block;
    void *new_ptr;

    if (!ptr)
        return os_malloc(size);

    if (size == 0) {
        os_free(ptr);
        return NULL;
    }

    block = PAYLOAD_TO_BLOCK(ptr);
    if (block->status == STATUS_FREE)
        return NULL;

    size = ALIGN(size);

    if (block->status == STATUS_ALLOC && META_SIZE + size < MMAP_THRESHOLD) {
        if (size <= block->size) {
            block_split(block, size);
            return ptr;
        }

        if (heap_try_expand(block, size))
            return ptr;
    }

    new_ptr = os_malloc(size);
    memcpy(new_ptr, ptr, block->size < size ? block->size : size);
    os_free(ptr);

    return new_ptr;
}
