#include "block.h"

/* heap blocks only, kept in address order; mmap blocks are not tracked */
static struct block_meta *head;
static struct block_meta *tail;

struct block_meta *block_list_head(void) {
    return head;
}

struct block_meta *block_list_tail(void) {
    return tail;
}

void block_list_append(struct block_meta *block) {
    block->next = NULL;
    block->prev = tail;

    if (tail)
        tail->next = block;
    else
        head = block;

    tail = block;
}

void block_split(struct block_meta *block, size_t size) {
    struct block_meta *rest;

    if (block->size < size + MIN_BLOCK_SIZE)
        return;

    rest = (struct block_meta *)((char *)block + META_SIZE + size);
    rest->size = block->size - size - META_SIZE;
    rest->status = STATUS_FREE;
    rest->prev = block;
    rest->next = block->next;

    if (block->next)
        block->next->prev = rest;
    else
        tail = rest;

    block->next = rest;
    block->size = size;
}

void block_merge_next(struct block_meta *block) {
    struct block_meta *next = block->next;

    block->size += META_SIZE + next->size;
    block->next = next->next;

    if (next->next)
        next->next->prev = block;
    else
        tail = block;
}

void block_coalesce_all(void) {
    struct block_meta *block = head;

    while (block) {
        /* don't advance after a merge: the new next may be free too */
        if (block->status == STATUS_FREE && block->next &&
            block->next->status == STATUS_FREE)
            block_merge_next(block);
        else
            block = block->next;
    }
}

struct block_meta *block_find_best(size_t size) {
    struct block_meta *best = NULL;

    for (struct block_meta *block = head; block; block = block->next) {
        if (block->status != STATUS_FREE || block->size < size)
            continue;

        if (!best || block->size < best->size)
            best = block;
    }

    return best;
}
