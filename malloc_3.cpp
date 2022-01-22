//
// Created by student on 1/20/22.
//

#include <unistd.h>
#include <iostream>
#include "malloc_2.h"

struct MallocMetadata{
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

static const int MAX_SMALLOC_SIZE = 100000000;
static const size_t BIN_SIZE = 1024;

static const int NUMBER_OF_BINS = 128;

struct MallocMetadata* _BLOCKS[NUMBER_OF_BINS] = {NULL};
struct MallocMetadata* _LAST[NUMBER_OF_BINS] = {NULL};

bool invalid_size(size_t size);

bool sbrk_success(const void *sbrk_res);

void *undo_list_init(size_t i);

void allocate_first_block(size_t size);

bool no_blocks_allocated(unsigned long i);

void *init_blocks_list(size_t size);

MallocMetadata *allocate_metadata();

void *allocate_new_block(size_t size);

void *undo_new_block(size_t i);

void init_new_block(size_t size);

void *repupose_block(MallocMetadata *block);

bool block_fits(const MallocMetadata *block, size_t wanted);

void set_zeroes(const void *new_block, size_t num, size_t size);

void copy_data(const void *from, const void *to, size_t size);

void* smalloc(size_t size){
    if (invalid_size(size)) return SMALLOC_FAIL_VALUE;

//    if (no_blocks_allocated()) return init_blocks_list(size);

    for(int bin = size / BIN_SIZE; bin < NUMBER_OF_BINS; ++bin){
        for (MallocMetadata* iter = _BLOCKS[bin]; iter != NULL; iter = iter->next)
            if(block_fits(iter, size)) return repupose_block(iter);
    }
        

    return allocate_new_block(size);
}

bool block_fits(const MallocMetadata *block, size_t wanted) { return block->is_free && block->size >= wanted; }

void *repupose_block(MallocMetadata *block) {
    block->is_free = false;
    return block;
}

void sfree(void *p) {
    struct MallocMetadata* pmd = (MallocMetadata*) p;
    pmd->is_free = true;
}

size_t _size_meta_data() {
    return sizeof(struct MallocMetadata);
}

size_t _num_allocated_blocks() {
    size_t counter = 0;
    for(int bin = 0; bin < NUMBER_OF_BINS; ++bin)
        for (MallocMetadata *iter = _BLOCKS[bin]; iter != NULL; iter = iter->next) ++counter;
    return counter;
}

size_t _num_meta_data_bytes() {
    return _num_allocated_blocks() * _size_meta_data();
}

size_t _num_allocated_bytes() {
    size_t counter = 0;
    for(int bin = 0; bin < NUMBER_OF_BINS; ++bin)
        for (MallocMetadata* iter = _BLOCKS[bin]; iter != NULL ; iter = iter->next) counter += iter->size;

    return counter;
}

size_t _num_free_blocks() {
    size_t counter = 0;
    for(int bin = 0; bin < NUMBER_OF_BINS; ++bin)
        for (MallocMetadata* iter = _BLOCKS[bin]; iter != NULL ; iter = iter->next) counter += (iter->is_free);

    return counter;
}

size_t _num_free_bytes() {
    size_t counter = 0;
    for(int bin = 0; bin < NUMBER_OF_BINS; ++bin)
        for (MallocMetadata* iter = _BLOCKS[bin]; iter != NULL ; iter = iter->next)
            counter += (iter->is_free) * iter->size;
    return counter;
}

void *scalloc(size_t num, size_t size) {
    void* new_block = smalloc(num * size);
    if (new_block == SMALLOC_FAIL_VALUE) return SMALLOC_FAIL_VALUE;
    set_zeroes(new_block, num, size);
    return new_block;
}

void *srealloc(void *oldp, size_t size) {
    if (oldp == NULL) return allocate_new_block(size);
    struct MallocMetadata* mdp = (MallocMetadata*) oldp;
    if (size <= mdp->size) return oldp;
    void* out = smalloc(size);
    if(out == SMALLOC_FAIL_VALUE) return SMALLOC_FAIL_VALUE;
    copy_data(oldp, out, size);
    sfree(oldp);
    return out;
}

void copy_data(const void *from, const void *to, size_t size) {
    struct MallocMetadata* mdp = (MallocMetadata*) from;
    for (size_t i = 0; i < size && i < mdp->size; ++i) *((char*)to + _size_meta_data() + i) = *((char*)from + _size_meta_data() + i);
}

void set_zeroes(const void *new_block, size_t num, size_t size) { for (size_t i = 0; i < num * size; ++i) *((char*)new_block + _size_meta_data() + i) = 0; }

void *allocate_new_block(size_t size) {
    size_t bin = size / BIN_SIZE;
    if (no_blocks_allocated(bin)) return init_blocks_list(size);
    _LAST[bin]->next = allocate_metadata();
    if (!sbrk_success(_LAST[bin]->next)) return undo_new_block(bin);
    init_new_block(size);
    return sbrk_success(sbrk(size)) ? _LAST : SMALLOC_FAIL_VALUE;
}

void init_new_block(size_t size) {
    size_t bin = size / BIN_SIZE;
    _LAST[bin]->next->prev = _LAST[bin];
    _LAST[bin] = _LAST[bin]->next;
    _LAST[bin]->next = NULL;
    _LAST[bin]->is_free = false;
    _LAST[bin]->size = size;
}

void *undo_new_block(size_t i) {
    _LAST[i]->next = NULL;
    return SMALLOC_FAIL_VALUE;
}

void *init_blocks_list(size_t size) {
    size_t bin = size / BIN_SIZE;
    _BLOCKS[bin] = allocate_metadata();
    if (!sbrk_success(_BLOCKS)) return undo_list_init(bin);
    allocate_first_block(size);
    if (!sbrk_success(sbrk(size))) return undo_list_init(0);
    return _BLOCKS[bin];
}

MallocMetadata *allocate_metadata() { return (MallocMetadata*)(sbrk(_size_meta_data())); }

bool no_blocks_allocated(unsigned long i) { return _BLOCKS[i] == NULL; }

void allocate_first_block(size_t size) {
    size_t bin = size/BIN_SIZE;
    _LAST[bin] = _BLOCKS[bin];
    _BLOCKS[bin]->next = NULL;
    _BLOCKS[bin]->prev = NULL;
    _BLOCKS[bin]->size = size;
    _BLOCKS[bin]->is_free = false;
}

void *undo_list_init(size_t i) {
    _BLOCKS[i] = NULL;
    _LAST[i] = NULL;
    return SMALLOC_FAIL_VALUE;
}

bool sbrk_success(const void *sbrk_res) { return sbrk_res != (void*)-1; }

bool invalid_size(size_t size) { return !size || size > MAX_SMALLOC_SIZE; }