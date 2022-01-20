#include <iostream>
//#include "malloc_1.h"
#include "malloc_2.h"

void test_malloc_1();

void print_malloc_2_metrics();

int main() {
//    test_malloc_1();
    print_malloc_2_metrics();
    void *zero;
    zero = smalloc(0);
    std::cout << zero << std::endl;
    print_malloc_2_metrics();
    void *one;
    one = smalloc(1);
    std::cout << one << std::endl;
    print_malloc_2_metrics();
    void *too_big;
    too_big = smalloc(100000001);
    std::cout << too_big << std::endl;
    print_malloc_2_metrics();
    void *five;
    five = smalloc(5);
    std::cout << five << std::endl;
    print_malloc_2_metrics();
    void *max;
    max = smalloc(100000000);
    std::cout << max << std::endl;
    print_malloc_2_metrics();
    void *six;
    six = smalloc(6);
    std::cout << six << std::endl;
    print_malloc_2_metrics();

    sfree(five);
    print_malloc_2_metrics();
    void *five_again;
    five_again = smalloc(5);
    std::cout << five_again << std::endl;
    print_malloc_2_metrics();
    void* ten;
    ten = smalloc(10);
    std::cout << ten << std::endl;
    print_malloc_2_metrics();

    sfree(six);
    print_malloc_2_metrics();
    void *seven;
    seven = smalloc(7);
    std::cout << seven << std::endl;
    print_malloc_2_metrics();
    void *three;
    three = smalloc(3);
    std::cout << three << std::endl;
    print_malloc_2_metrics();

    sfree(five_again);
    sfree(three);
    print_malloc_2_metrics();
    three = smalloc(5);
    std::cout << three << std::endl;
    print_malloc_2_metrics();

    return 0;
}

void print_malloc_2_metrics() {
    std::cout << "Blocks Free: " << _num_free_blocks() << std::endl;
    std::cout << "Bytes Free: " << _num_free_bytes() << std::endl;
    std::cout << "Blocks Allocated: " << _num_allocated_blocks() << std::endl;
    std::cout << "Usable Bytes Allocated: " << _num_allocated_bytes() << std::endl;
    std::cout << "Metadata Total Size: " << _num_meta_data_bytes() << std::endl;
    std::cout << "Metadata Instance Size: " << _size_meta_data() << std::endl;
}

void test_malloc_1() {
    void *zero;
    zero = smalloc(0);
    std::cout << zero << std::endl;
    void *one;
    one = smalloc(1);
    std::cout << one << std::endl;
    void *too_big;
    too_big = smalloc(100000001);
    std::cout << too_big << std::endl;
    void *five;
    five = smalloc(5);
    std::cout << five << std::endl;
    void *max;
    max = smalloc(100000000);
    std::cout << max << std::endl;
    void *six;
    six = smalloc(6);
    std::cout << six << std::endl;
}
