//
// Created by student on 1/20/22.
//

#ifndef OS_234123_HW4_MALLOC_2_H
#define OS_234123_HW4_MALLOC_2_H
static void *const SMALLOC_FAIL_VALUE = NULL;

#include <unistd.h>


void* smalloc(size_t size);

void sfree(void* p);


size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();




#endif //OS_234123_HW4_MALLOC_2_H
