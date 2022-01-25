//
// Created by student on 1/20/22.
//

#include <unistd.h>
#include <iostream>

static void *const SMALLOC_FAIL_VALUE = NULL;


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

void *repurposeBlock(MallocMetadata *block);

bool block_fits(const MallocMetadata *block, size_t wanted);

void set_zeroes(const void *new_block, size_t num, size_t size);

void copy_data(const void *from, const void *to, size_t size);

void* smalloc(size_t size){
    if (invalid_size(size)) return SMALLOC_FAIL_VALUE;

    if (no_blocks_allocated()) return init_blocks_list(size);

    for (MallocMetadata* iter = _BLOCKS; iter != NULL; iter = iter->next)
        if(block_fits(iter, size)) return repurposeBlock(iter);

    return allocate_new_block(size);
}

bool block_fits(const MallocMetadata *block, size_t wanted) { return block->is_free && block->size >= wanted; }

size_t _size_meta_data() {
    return sizeof(struct MallocMetadata);
}

void *repurposeBlock(MallocMetadata *block) {
    block->is_free = false;
    return ((char*)block + _size_meta_data());
}

void sfree(void *p) {
    if (p != NULL) {
        struct MallocMetadata *pmd = (MallocMetadata *) ((char *) p - _size_meta_data());
        pmd->is_free = true;
    }
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
    if (size <= 0) return SMALLOC_FAIL_VALUE;
    if (oldp == NULL) return smalloc(size);
    struct MallocMetadata* mdp = (MallocMetadata*) ((char*)oldp - _size_meta_data());
    if (size <= mdp->size) return oldp;
    void* out = smalloc(size);
    if(out == SMALLOC_FAIL_VALUE) return SMALLOC_FAIL_VALUE;
    copy_data(oldp, out, size);
    sfree(oldp);
    return out;
}

void copy_data(const void *from, const void *to, size_t size) {
    struct MallocMetadata* mdp = (MallocMetadata*) ((char*)from - _size_meta_data());
    for (size_t i = 0; i < size && i < mdp->size; ++i) *((char*)to + i) = *((char*)from + i);
}

void set_zeroes(const void *new_block, size_t num, size_t size) { for (size_t i = 0; i < num * size; ++i) *((char*)new_block + i) = 0; }

void *allocate_new_block(size_t size) {
    if (no_blocks_allocated()){
        return init_blocks_list(size);
    }
    else{
        _LAST->next = allocate_metadata();
        if (!sbrk_success(_LAST->next)) return undo_new_block();
        init_new_block(size);
        return sbrk_success(sbrk(size)) ? ((char *) _LAST + _size_meta_data()) : SMALLOC_FAIL_VALUE;
    }
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
    return ((char*)_BLOCKS + _size_meta_data());
}

MallocMetadata *allocate_metadata() { return (MallocMetadata*)(sbrk(_size_meta_data())); }

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