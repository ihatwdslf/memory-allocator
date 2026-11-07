#pragma once

#include <stddef.h>
#include <stdlib.h>
#include "block_meta.h"

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~((size_t)ALIGNMENT - 1))

#define META_SIZE ALIGN(sizeof(struct block_meta))
#define MMAP_THRESHOLD (128 * 1024)
#define HEAP_PREALLOC_SIZE (128 * 1024)

/* smallest block worth splitting off: header + 8 bytes of payload */
#define MIN_BLOCK_SIZE (META_SIZE + ALIGN(1))

#define BLOCK_TO_PAYLOAD(block) ((void *)((char *)(block) + META_SIZE))
#define PAYLOAD_TO_BLOCK(ptr) ((struct block_meta *)((char *)(ptr) - META_SIZE))
