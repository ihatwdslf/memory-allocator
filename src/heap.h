#pragma once

#include "common.h"

struct block_meta *heap_alloc_block(size_t size);
int heap_try_expand(struct block_meta *block, size_t size);

struct block_meta *mmap_alloc_block(size_t size);
void mmap_free_block(struct block_meta *block);
