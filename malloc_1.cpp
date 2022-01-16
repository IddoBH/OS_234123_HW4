//
// Created by student on 1/16/22.
//
#include <unistd.h>

static const int MAX_SMALLOC_SIZE = 100000000;

bool invalid_size(size_t size);

bool sbrk_success(const void *sbrk_res);

void* smalloc(size_t size){
    if (invalid_size(size)) return NULL;
    void *sbrk_res = sbrk(size);
    return sbrk_success(sbrk_res)? sbrk_res : NULL;
}

bool sbrk_success(const void *sbrk_res) { return sbrk_res != (void*)-1; }

bool invalid_size(size_t size) { return !size || size > MAX_SMALLOC_SIZE; }

