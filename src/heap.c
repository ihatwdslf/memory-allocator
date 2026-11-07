#include <unistd.h>
#include <sys/mman.h>

#include "heap.h"
#include "block.h"

static int heap_preallocated;

static void *heap_grow(size_t increment) {
    void *area = sbrk(increment);

    DIE(area == (void *)-1, "sbrk");
    return area;
}

static void heap_preallocate(void) {
    struct block_meta *block = heap_grow(HEAP_PREALLOC_SIZE);

    block->size = HEAP_PREALLOC_SIZE - META_SIZE;
    block->status = STATUS_FREE;
    block_list_append(block);

    heap_preallocated = 1;
}

struct block_meta *heap_alloc_block(size_t size) {
    struct block_meta *block;

    if (!heap_preallocated)
        heap_preallocate();

    block_coalesce_all();
    block = block_find_best(size);
    if (block) {
        block_split(block, size);
        block->status = STATUS_ALLOC;
        return block;
    }

    /* last block is free but too small - extend it instead of adding a new one */
    block = block_list_tail();
    if (block && block->status == STATUS_FREE) {
        heap_grow(size - block->size);
        block->size = size;
        block->status = STATUS_ALLOC;
        return block;
    }

    block = heap_grow(META_SIZE + size);
    block->size = size;
    block->status = STATUS_ALLOC;
    block_list_append(block);

    return block;
}

int heap_try_expand(struct block_meta *block, size_t size) {
    /* absorb free neighbours one at a time, they stay merged even on failure */
    while (block->next && block->next->status == STATUS_FREE) {
        block_merge_next(block);

        if (block->size >= size) {
            block_split(block, size);
            return 1;
        }
    }

    if (!block->next) {
        heap_grow(size - block->size);
        block->size = size;
        return 1;
    }

    return 0;
}

struct block_meta *mmap_alloc_block(size_t size) {
    struct block_meta *block;

    block = mmap(NULL, META_SIZE + size, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    DIE(block == MAP_FAILED, "mmap");

    block->size = size;
    block->status = STATUS_MAPPED;
    block->prev = NULL;
    block->next = NULL;

    return block;
}

void mmap_free_block(struct block_meta *block) {
    int ret = munmap(block, META_SIZE + block->size);

    DIE(ret == -1, "munmap");
}
