#pragma once

#include "common.h"

struct block_meta *block_list_head(void);
struct block_meta *block_list_tail(void);
void block_list_append(struct block_meta *block);

void block_split(struct block_meta *block, size_t size);
void block_merge_next(struct block_meta *block);
void block_coalesce_all(void);
struct block_meta *block_find_best(size_t size);
