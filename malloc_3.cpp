//
// Created by student on 1/20/22.
//

#include <unistd.h>
#include <iostream>
#include <sys/mman.h>

static void *const SMALLOC_FAIL_VALUE = NULL;


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

struct MallocMetadata* _BLOCKS_XL = NULL;
struct MallocMetadata* _LAST_XL = NULL;

bool invalid_size(size_t size);

bool sbrk_success(const void *sbrk_res);

void *undo_list_init();

void allocate_first_block(size_t size);

bool no_blocks_allocated(MallocMetadata *blocks_list);

void *init_blocks_list(size_t size);

MallocMetadata *allocate_metadata();

void *allocate_new_block(size_t size);

void *undo_new_block();

void init_new_block(size_t size);

void *repurposeBlock(MallocMetadata *block, size_t wanted);

bool block_fits(const MallocMetadata* block, size_t wanted);

void set_zeroes(const void *new_block, size_t num, size_t size);

void copy_data(const void *from, const void *to, size_t size);

void *init_blocks_list_and_size_table(size_t size);

void init_bin(size_t bin, MallocMetadata *new_block);

void push_new_block_before_bin(size_t bin, MallocMetadata *new_block);

void push_new_block_mid_bin(MallocMetadata *iter, MallocMetadata *new_block);

void push_new_block_after_bin(size_t bin, MallocMetadata *new_block);

void place_new_block_in_correct_order(size_t bin, MallocMetadata *new_block);

void insert_block_to_size_bins(MallocMetadata *new_block);

MallocMetadata *seperate_block(const MallocMetadata *block, size_t wanted);

bool large_enough(size_t remaining_space);

void init_residue(size_t remaining_space, MallocMetadata *residue);

void push_residue_to_block_list(MallocMetadata *block, MallocMetadata *residue);

void remove_old_block_from_bins(MallocMetadata *block, size_t full);

void divide_block(MallocMetadata *block, size_t wanted, size_t full, size_t remaining_space);

void unite_adjacent_blocks(MallocMetadata *first, MallocMetadata *second);

void unite_free_blocks_on_both_sides(MallocMetadata *pmd);

void *expand_last_block(size_t size);


bool small_block(size_t size);

bool two_blocks_fit(size_t size, const MallocMetadata *block1, MallocMetadata *block2);

void *use_same_block(void *oldp, size_t size, MallocMetadata *mdp);

void *use_same_block_with_prev(size_t size, MallocMetadata *mdp);

void *use_same_block_with_next(size_t size, MallocMetadata *mdp);

void *use_3_blocks(size_t size, MallocMetadata *mdp);

size_t _size_meta_data() {
    return sizeof(struct MallocMetadata);
}

size_t _num_allocated_blocks() {
    size_t counter = 0;
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next){
        ++counter;
    }
    for (MallocMetadata* iter = _BLOCKS_XL; iter != NULL ; iter = iter->next) {
        ++counter;
    }
    return counter;
}

size_t _num_meta_data_bytes() {
    return _num_allocated_blocks() * _size_meta_data();
}

size_t _num_allocated_bytes() {
    size_t counter = 0;
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next) counter += iter->size;
    for (MallocMetadata* iter = _BLOCKS_XL; iter != NULL ; iter = iter->next) counter += iter->size;
    return counter;
}

size_t _num_free_blocks() {
    size_t counter = 0;
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next) counter += (iter->is_free);
    for (MallocMetadata* iter = _BLOCKS_XL; iter != NULL ; iter = iter->next) counter += (iter->is_free);
    return counter;
}

size_t _num_free_bytes() {
    size_t counter = 0;
    for (MallocMetadata* iter = _BLOCKS; iter != NULL ; iter = iter->next) counter += (iter->is_free) * iter->size;
    for (MallocMetadata* iter = _BLOCKS_XL; iter != NULL ; iter = iter->next) counter += (iter->is_free) * iter->size;
    return counter;
}

void* smalloc(size_t size){
    if (invalid_size(size)) return SMALLOC_FAIL_VALUE;

    for(size_t bin = size / BIN_SIZE; bin < SIZE_BINS_NUMBER; ++bin)
        for (MallocMetadata* iter = _BLOCKS_BY_SIZE[bin]; iter != NULL; iter = iter->next_size)
            if(block_fits(iter, size)) return repurposeBlock(iter, size);

    return allocate_new_block(size);
}

bool block_fits(const MallocMetadata *block, size_t wanted) { return block->is_free && block->size >= wanted; }

void *repurposeBlock(MallocMetadata *block, size_t wanted) {
    block->is_free = false;
    size_t full = block->size;
    size_t remaining_space = full - wanted;
    if (large_enough(remaining_space)) divide_block(block, wanted, full, remaining_space);
    return block + _size_meta_data();
}

void divide_block(MallocMetadata *block, size_t wanted, size_t full, size_t remaining_space) {
    MallocMetadata *residue = seperate_block(block, wanted);
    init_residue(remaining_space, residue);

    block->size = wanted;

    push_residue_to_block_list(block, residue);

    remove_old_block_from_bins(block, full);

    insert_block_to_size_bins(block);
    insert_block_to_size_bins(residue);
}

void remove_old_block_from_bins(MallocMetadata *block, size_t full) {
    size_t bin = full / BIN_SIZE;
    if (block->next_size == NULL){
        if (block->prev_size == NULL) _BLOCKS_BY_SIZE[bin] = NULL;
        else block->prev_size->next_size = NULL;
    } else{
        if (block->prev_size == NULL) {
            _BLOCKS_BY_SIZE[bin] = block->next_size;
            block->next_size->prev_size = NULL;
        }
        else{
            block->prev_size->next_size = block->next_size;
            block->next_size->prev_size = block->prev_size;
        }
    }
}

void push_residue_to_block_list(MallocMetadata *block, MallocMetadata *residue) {
    residue->next = block->next;
    if(block->next != NULL) block->next->prev = residue;
    else _LAST = residue;
    residue->prev = block;
    block->next = residue;
}

void init_residue(size_t remaining_space, MallocMetadata *residue) {
    residue->is_free = true;
    residue->size = remaining_space - _size_meta_data();
}

bool large_enough(size_t remaining_space) { return remaining_space >= 128 + _size_meta_data(); }

MallocMetadata *seperate_block(const MallocMetadata *block, size_t wanted) {
    void * new_block = ((char*)block + _size_meta_data() + wanted);
    MallocMetadata* residue = (MallocMetadata*)new_block;
    return residue;
}

void sfree(void *p) {
    struct MallocMetadata* pmd = (MallocMetadata*) ((char*)p - _size_meta_data());
    if (small_block(pmd->size)){
//        if (pmd->next == NULL){
//            if(pmd->prev == NULL){
//                undo_list_init();
//            }else{
//                pmd->prev->next = NULL;
//                _LAST = pmd->prev;
//            }
//            remove_old_block_from_bins(pmd, pmd->size);
//            sbrk(-(pmd->size));
//            pmd->size = 0;
//            pmd->is_free = true;
//            return;
//        }
        pmd->is_free = true;
        unite_free_blocks_on_both_sides(pmd);
    } else{
        if (pmd->next == NULL){
            if (pmd->prev == NULL) _BLOCKS_XL = NULL;
            else pmd->prev->next = NULL;
        } else{
            if (pmd->prev == NULL) {
                _BLOCKS_XL = pmd->next;
                pmd->next->prev = NULL;
            }
            else{
                pmd->prev->next = pmd->next;
                pmd->next->prev = pmd->prev;
            }
        }
        munmap(p, pmd->size + _size_meta_data());
    }
}

bool small_block(size_t size) { return size < BIN_SIZE * SIZE_BINS_NUMBER; }

void unite_free_blocks_on_both_sides(MallocMetadata *pmd) {
    if(pmd->next != NULL && pmd->next->is_free) unite_adjacent_blocks(pmd, pmd->next);
    if(pmd->prev != NULL && pmd->prev->is_free) unite_adjacent_blocks(pmd->prev, pmd);
}

void unite_adjacent_blocks(MallocMetadata *first, MallocMetadata *second) {
    size_t first_size = first->size;
    size_t second_size = second->size;

    first->size += second->size + _size_meta_data();
    first->next = second->next;

    if (second->next != NULL) second->next->prev = first;
    else _LAST = first;

    remove_old_block_from_bins(first, first_size);
    remove_old_block_from_bins(second, second_size);

    insert_block_to_size_bins(first);
}

void *scalloc(size_t num, size_t size) {
    void* new_block = smalloc(num * size);
    if (new_block == SMALLOC_FAIL_VALUE) return SMALLOC_FAIL_VALUE;
    set_zeroes(new_block, num, size);
    return new_block;
}

void *srealloc(void *oldp, size_t size) {
    if (oldp == NULL){
        for(size_t bin = size / BIN_SIZE; bin < SIZE_BINS_NUMBER; ++bin)
            for (MallocMetadata* iter = _BLOCKS_BY_SIZE[bin]; iter != NULL; iter = iter->next_size)
                if(block_fits(iter, size)) return repurposeBlock(iter, size);

        return allocate_new_block(size);
    } return allocate_new_block(size);
    struct MallocMetadata* mdp = (MallocMetadata*) oldp;
    if (size <= mdp->size)
        return use_same_block(oldp, size, mdp);
    if (two_blocks_fit(size, mdp, mdp->prev) && !two_blocks_fit(size, mdp, mdp->next))
        return use_same_block_with_prev(size, mdp);
    if (two_blocks_fit(size, mdp, mdp->next) && !two_blocks_fit(size, mdp, mdp->prev))
        return use_same_block_with_next(size, mdp);
    if (mdp->prev != NULL && mdp->prev->is_free && mdp->next != NULL && mdp->next->is_free && size <= (mdp->size + mdp->prev->size + mdp->next->size + 2*_size_meta_data()))
        return use_3_blocks(size, mdp);
    if (mdp->next == NULL) return expand_last_block(size);
    void* out = smalloc(size);
    if(out == SMALLOC_FAIL_VALUE) return SMALLOC_FAIL_VALUE;
    copy_data(oldp, out, size);
    sfree(oldp);
    return out;
}

void *use_3_blocks(size_t size, MallocMetadata *mdp) {
    void* out = mdp->prev;
    unite_free_blocks_on_both_sides(mdp);
    MallocMetadata* out_md = (MallocMetadata*)out;
    out_md->is_free = false;
    size_t full = mdp->prev->size;
    size_t remaining_space = full - size;
    if (large_enough(remaining_space)) divide_block(mdp->prev, size, full, remaining_space);
    return out;
}

void *use_same_block_with_next(size_t size, MallocMetadata *mdp) {
    unite_adjacent_blocks(mdp, mdp->next);
    mdp->is_free = false;
    size_t full = mdp->size;
    size_t remaining_space = full - size;
    if (large_enough(remaining_space)) divide_block(mdp, size, full, remaining_space);
    return mdp;
}

void *use_same_block_with_prev(size_t size, MallocMetadata *mdp) {
    unite_adjacent_blocks(mdp->prev, mdp);
    copy_data(mdp, mdp->prev, size);
    mdp->prev->is_free = false;
    size_t full = mdp->prev->size;
    size_t remaining_space = full - size;
    if (large_enough(remaining_space)) divide_block(mdp->prev, size, full, remaining_space);
    return mdp->prev;
}

void *use_same_block(void *oldp, size_t size, MallocMetadata *mdp) {
    size_t full = mdp->size;
    size_t remaining_space = full - size;
    if (large_enough(remaining_space)) divide_block(mdp, size, full, remaining_space);
    return oldp;
}

bool two_blocks_fit(size_t size, const MallocMetadata *block1, MallocMetadata *block2) {
    return block2 != NULL && block2->is_free && size <= block1->size + block2->size + _size_meta_data();
}

void copy_data(const void *from, const void *to, size_t size) {
    struct MallocMetadata* mdp = (MallocMetadata*) from;
    for (size_t i = 0; i < size && i < mdp->size; ++i) *((char*)to + _size_meta_data() + i) = *((char*)from + _size_meta_data() + i);
}

void set_zeroes(const void *new_block, size_t num, size_t size) { for (size_t i = 0; i < num * size; ++i) *((char*)new_block + _size_meta_data() + i) = 0; }

void *allocate_new_block(size_t size) {
    if (small_block(size)){
        if (no_blocks_allocated(_BLOCKS)) return init_blocks_list_and_size_table(size);

        if (_LAST->is_free) return expand_last_block(size);

        _LAST->next = allocate_metadata();
        if (!sbrk_success(_LAST->next)) return undo_new_block();
        init_new_block(size);
        return sbrk_success(sbrk(size)) ? _LAST + _size_meta_data() : SMALLOC_FAIL_VALUE;
    }else{
        void *out =
                mmap(NULL, size + _size_meta_data(), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
        if(out == MAP_FAILED) return NULL;
        MallocMetadata* pmd = (MallocMetadata*)out;
        pmd->size = size;
        pmd->is_free = false;
        pmd->next_size = NULL;
        pmd->prev_size = NULL;
        if (no_blocks_allocated(_BLOCKS_XL)){
            _BLOCKS_XL = pmd;
            _LAST_XL = _BLOCKS_XL;
            pmd->next = NULL;
            pmd->prev = NULL;
        } else{
            _LAST_XL->next = pmd;
            pmd->next = NULL;
            pmd->prev = _LAST_XL;
            _LAST_XL = pmd;
        }
        return (char*)out + _size_meta_data();
    }
}

void *expand_last_block(size_t size) {
    if(!sbrk_success(sbrk(size - _LAST->size))) return SMALLOC_FAIL_VALUE;

    remove_old_block_from_bins(_LAST, _LAST->size);
    _LAST->size = size;
    _LAST->is_free = false;
    insert_block_to_size_bins(_LAST);

    return _LAST + _size_meta_data();
}

void *init_blocks_list_and_size_table(size_t size) {
    void *out = init_blocks_list(size);
    size_t bin = size / BIN_SIZE;
    _BLOCKS_BY_SIZE[bin] = (MallocMetadata*)out;
    _LAST_BY_SIZE[bin] = _BLOCKS_BY_SIZE[bin];
    return (char*)out + _size_meta_data();
}

void init_new_block(size_t size) {
    _LAST->next->prev = _LAST;
    _LAST = _LAST->next;
    _LAST->next = NULL;
    _LAST->is_free = false;
    _LAST->size = size;
    insert_block_to_size_bins(_LAST);
}

void insert_block_to_size_bins(MallocMetadata *new_block) {
    if(_BLOCKS_BY_SIZE[new_block->size/BIN_SIZE] == NULL) init_bin(new_block->size/BIN_SIZE, new_block);
    else if(_BLOCKS_BY_SIZE[new_block->size/BIN_SIZE]->size > new_block->size) push_new_block_before_bin(new_block->size/BIN_SIZE, new_block);
    else place_new_block_in_correct_order(new_block->size/BIN_SIZE, new_block);
}

void place_new_block_in_correct_order(size_t bin, MallocMetadata *new_block) {
    for(MallocMetadata* iter = _BLOCKS_BY_SIZE[bin]; iter != NULL; iter = iter->next_size)
        if(iter->size > new_block->size) {
            push_new_block_mid_bin(iter, new_block);
            return;
        }
    push_new_block_after_bin(bin, new_block);
}

void push_new_block_after_bin(size_t bin, MallocMetadata *new_block) {
    new_block->prev_size = _LAST_BY_SIZE[bin];
    new_block->next_size = NULL;
    _LAST_BY_SIZE[bin]->next_size = new_block;
    _LAST_BY_SIZE[bin] = new_block;
}

void push_new_block_mid_bin(MallocMetadata *iter, MallocMetadata *new_block) {
    iter->prev_size->next_size = new_block;
    new_block->prev_size = iter->prev_size;
    iter->prev_size = new_block;
    new_block->next_size = iter;
}

void push_new_block_before_bin(size_t bin, MallocMetadata *new_block) {
    new_block->prev_size = NULL;
    _BLOCKS_BY_SIZE[bin]->prev_size = new_block;
    new_block->next_size = _BLOCKS_BY_SIZE[bin];
    _BLOCKS_BY_SIZE[bin] = new_block;
}

void init_bin(size_t bin, MallocMetadata *new_block) {
    _BLOCKS_BY_SIZE[bin] = new_block;
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

bool no_blocks_allocated(MallocMetadata *blocks_list) { return blocks_list == NULL; }

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