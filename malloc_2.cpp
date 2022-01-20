//
// Created by student on 1/20/22.
//

#include <unistd.h>
#include "malloc_2.h"

struct MallocMetadata{
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

struct MallocMetadata* _BLOCKS = NULL;
struct MallocMetadata* _LAST = NULL;


static const int MAX_SMALLOC_SIZE = 100000000;

bool invalid_size(size_t size);

bool sbrk_success(const void *sbrk_res);

void *undo_list_init();

void allocate_first_block(size_t size);

bool no_blocks_allocated();

void *init_blocks_list(size_t size);

MallocMetadata *allocate_metadata();

void *allocate_new_block(size_t size);

void *undo_new_block();

void init_new_block(size_t size);

void *repupose_block(MallocMetadata *block);

bool block_fits(const MallocMetadata *block, size_t wanted);

void* smalloc(size_t size){
    if (invalid_size(size)) return SMALLOC_FAIL_VALUE;

    if (no_blocks_allocated()) return init_blocks_list(size);

    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next)
        if(block_fits(iter, size)) return repupose_block(iter);

    return allocate_new_block(size);
}

bool block_fits(const MallocMetadata *block, size_t wanted) { return block->is_free && block->size >= wanted; }

void *repupose_block(MallocMetadata *block) {
    block->is_free = false;
    return block;
}

void sfree(void *p) {
    MallocMetadata* pmd = (MallocMetadata*) p;
    pmd->is_free = true;
}

void *allocate_new_block(size_t size) {
    _LAST->next = allocate_metadata();
    if (!sbrk_success(_LAST->next)) return undo_new_block();
    init_new_block(size);
    return sbrk_success(sbrk(size)) ? _LAST : SMALLOC_FAIL_VALUE;
}

void init_new_block(size_t size) {
    _LAST->next->prev = _LAST;
    _LAST = _LAST->next;
    _LAST->next = NULL;
    _LAST->is_free = false;
    _LAST->size = size;
}

void *undo_new_block() {
    _LAST->next = NULL;
    return SMALLOC_FAIL_VALUE;
}

void *init_blocks_list(size_t size) {
    _BLOCKS = allocate_metadata();
    if (!sbrk_success(_BLOCKS)) return undo_list_init();
    allocate_first_block(size);
    if (!sbrk_success(sbrk(size))) return undo_list_init();
    return _BLOCKS;
}

MallocMetadata *allocate_metadata() { return (MallocMetadata*)(sbrk(sizeof(struct MallocMetadata))); }

bool no_blocks_allocated() { return _BLOCKS == NULL; }

void allocate_first_block(size_t size) {
    _LAST = _BLOCKS;
    _BLOCKS->next = NULL;
    _BLOCKS->prev = NULL;
    _BLOCKS->size = size;
    _BLOCKS->is_free = false;
}

void *undo_list_init() {
    _BLOCKS = NULL;
    _LAST = NULL;
    return SMALLOC_FAIL_VALUE;
}

bool sbrk_success(const void *sbrk_res) { return sbrk_res != (void*)-1; }

bool invalid_size(size_t size) { return !size || size > MAX_SMALLOC_SIZE; }