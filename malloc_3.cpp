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
    MallocMetadata* next_size;
    MallocMetadata* prev_size;
};
static const size_t MAX_SMALLOC_SIZE = 100000000;
static const size_t SIZE_BINS_NUMBER = 128;
static const size_t BIN_SIZE = 1024;

struct MallocMetadata* _BLOCKS_BY_SIZE[SIZE_BINS_NUMBER] = {NULL};
struct MallocMetadata* _LAST_BY_SIZE[SIZE_BINS_NUMBER] = {NULL};

struct MallocMetadata* _BLOCKS = NULL;
struct MallocMetadata* _LAST = NULL;


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

void *repurposeBlock(MallocMetadata *block);

bool block_fits(const MallocMetadata *block, size_t wanted);

void set_zeroes(const void *new_block, size_t num, size_t size);

void copy_data(const void *from, const void *to, size_t size);

void *init_blocks_list_and_size_table(size_t size);

void init_bin(size_t bin);

void push_new_block_before_bin(size_t bin);

void push_new_block_mid_bin(MallocMetadata *iter);

void push_new_block_after_bin(size_t bin);

void place_new_block_in_correct_order(size_t bin);

void* smalloc(size_t size){
    if (invalid_size(size)) return SMALLOC_FAIL_VALUE;

    for(size_t bin = size / BIN_SIZE; bin < SIZE_BINS_NUMBER; ++bin)
        for (MallocMetadata* iter = _BLOCKS_BY_SIZE[bin]; iter != NULL; iter = iter->next)
            if(block_fits(iter, size)) return repurposeBlock(iter);

    return allocate_new_block(size);
}

bool block_fits(const MallocMetadata *block, size_t wanted) { return block->is_free && block->size >= wanted; }

void *repurposeBlock(MallocMetadata *block) {
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
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next) ++counter;
    return counter;
}

size_t _num_meta_data_bytes() {
    return _num_allocated_blocks() * _size_meta_data();
}

size_t _num_allocated_bytes() {
    size_t counter = 0;
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next){
        counter += iter->size;
    }
    return counter;
}

size_t _num_free_blocks() {
    size_t counter = 0;
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next){
        counter += (iter->is_free);
    }
    return counter;
}

size_t _num_free_bytes() {
    size_t counter = 0;
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next){
        counter += (iter->is_free) * iter->size;
    }
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
    if (no_blocks_allocated()) return init_blocks_list_and_size_table(size);
    _LAST->next = allocate_metadata();
    if (!sbrk_success(_LAST->next)) return undo_new_block();
    init_new_block(size);
    return sbrk_success(sbrk(size)) ? _LAST : SMALLOC_FAIL_VALUE;
}

void *init_blocks_list_and_size_table(size_t size) {
    void *out = init_blocks_list(size);
    size_t bin = size / BIN_SIZE;
    _BLOCKS_BY_SIZE[bin] = (MallocMetadata*)out;
    _LAST_BY_SIZE[bin] = _BLOCKS_BY_SIZE[bin];
    return out;
}

void init_new_block(size_t size) {
    _LAST->next->prev = _LAST;
    _LAST = _LAST->next;
    _LAST->next = NULL;
    _LAST->is_free = false;
    _LAST->size = size;
    size_t bin = size / BIN_SIZE;
    if(_BLOCKS_BY_SIZE[bin] == NULL) init_bin(bin);
    else if(_BLOCKS_BY_SIZE[bin]->size > _LAST->size) push_new_block_before_bin(bin);
    else place_new_block_in_correct_order(bin);

}

void place_new_block_in_correct_order(size_t bin) {
    for(MallocMetadata* iter = _BLOCKS_BY_SIZE[bin]; iter != NULL; iter = iter->next_size)
        if(iter->size > _LAST->size) push_new_block_mid_bin(iter);
    push_new_block_after_bin(bin);
}

void push_new_block_after_bin(size_t bin) {
    _LAST->prev_size = _LAST_BY_SIZE[bin];
    _LAST->next_size = NULL;
    _LAST_BY_SIZE[bin]->next_size = _LAST;
    _LAST_BY_SIZE[bin] = _LAST;
}

void push_new_block_mid_bin(MallocMetadata *iter) {
    iter->prev_size->next_size = _LAST;
    _LAST->prev_size = iter->prev_size;
    iter->prev_size = _LAST;
    _LAST->next_size = iter;
}

void push_new_block_before_bin(size_t bin) {
    _LAST->prev_size = NULL;
    _BLOCKS_BY_SIZE[bin]->prev_size = _LAST;
    _LAST->next_size = _BLOCKS_BY_SIZE[bin];
    _BLOCKS_BY_SIZE[bin] = _LAST;
}

void init_bin(size_t bin) {
    _BLOCKS_BY_SIZE[bin] = _LAST;
    _BLOCKS_BY_SIZE[bin]->next_size = NULL;
    _BLOCKS_BY_SIZE[bin]->prev_size = NULL;
    _LAST_BY_SIZE[bin] = _BLOCKS_BY_SIZE[bin];
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

MallocMetadata *allocate_metadata() { return (MallocMetadata*)(sbrk(_size_meta_data())); }

bool no_blocks_allocated() { return _BLOCKS == NULL; }

void allocate_first_block(size_t size) {
    _LAST = _BLOCKS;
    _BLOCKS->next = NULL;
    _BLOCKS->prev = NULL;
    _BLOCKS->next_size = NULL;
    _BLOCKS->prev_size = NULL;
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