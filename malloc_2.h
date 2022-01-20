//
// Created by student on 1/20/22.
//

#ifndef OS_234123_HW4_MALLOC_2_H
#define OS_234123_HW4_MALLOC_2_H
static void *const SMALLOC_FAIL_VALUE = NULL;

#include <unistd.h>


void* smalloc(size_t size);

void sfree(void* p);


#endif //OS_234123_HW4_MALLOC_2_H
