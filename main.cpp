#include <iostream>
//#include "malloc_1.h"
//#include "malloc_2.h"
#include "malloc_3.h"

void test_malloc_1();

void print_malloc_2_metrics();

void test_malloc_2();

int main() {
//    test_malloc_1();
//    test_malloc_2();
    std::cout << std::endl << "Malloc 3:" << std::endl << std::endl;

    void *kb100 = scalloc(100,1024);
    void *kb20 = scalloc(20,1024);
    void *kb50 = scalloc(50,1024);
    sfree(kb100);
    sfree(kb50);
    sfree(kb20);
    print_malloc_2_metrics();
    void *kb30 = smalloc(30*1024);
    print_malloc_2_metrics();
    srealloc(kb30, 60*1024);
    print_malloc_2_metrics();

    void *kb90_500 = smalloc(90*1024+500);
    void *kb90_100 = smalloc(90*1024+100);
    void *kb90_300 = smalloc(90*1024+300);
    void *kb90_290 = smalloc(90*1024+290);
    print_malloc_2_metrics();
    sfree(kb90_500);
    sfree(kb90_100);
    sfree(kb90_300);
    print_malloc_2_metrics();
    size_t before = _num_free_bytes();
    void *kb90_280 = smalloc(90*1024+280);
    print_malloc_2_metrics();
    std::cout << "Used for realloc: " << (before - _num_free_bytes()) % 1024 << " bytes." << std::endl;



    return 0;
}

void test_malloc_2() {
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

    void* two_ints;
    two_ints = scalloc(2, sizeof(int));
    std::cout << two_ints << std::endl;
    print_malloc_2_metrics();
    for (int i = 0; i < 2; ++i) {
        std::cout << (int)*((char*)two_ints + _size_meta_data() + sizeof(int)*i) << std::endl;
    }
    *((char*)two_ints + _size_meta_data()) = 1;

    void* six_chars;
    six_chars = scalloc(6, sizeof(char));
    std::cout << six_chars << std::endl;
    print_malloc_2_metrics();
    for (int i = 0; i < 6; ++i) {
        std::cout << (char)*((char*)six_chars + _size_meta_data() + sizeof(char)*i) << std::endl;
    }

    sfree(max);
    print_malloc_2_metrics();
    void* bigger_ti = srealloc(two_ints, 500);
    print_malloc_2_metrics();
    for (int i = 0; i < 3; ++i) {
        std::cout << (int)*((char*)bigger_ti + _size_meta_data() + sizeof(int)*i) << std::endl;
    }

    void* four = scalloc(4, sizeof(char));
    *((char*)four + _size_meta_data() + sizeof(char)*0) = 'f';
    *((char*)four + _size_meta_data() + sizeof(char)*1) = 'o';
    *((char*)four + _size_meta_data() + sizeof(char)*2) = 'u';
    *((char*)four + _size_meta_data() + sizeof(char)*3) = 'r';
    for (int i = 0; i < 4; ++i) {
        std::cout << (char)*((char*)four + _size_meta_data() + sizeof(char)*i) << std::endl;
    }
    print_malloc_2_metrics();
    void* four_plus = srealloc(four, 100);
    print_malloc_2_metrics();
    for (int i = 0; i < 5; ++i) {
        std::cout << (char)*((char*)four_plus + _size_meta_data() + sizeof(char)*i) << std::endl;
    }
    four = srealloc(four_plus, 8);
    for (int i = 0; i < 5; ++i) {
        std::cout << (char)*((char*)four + _size_meta_data() + sizeof(char)*i) << std::endl;
    }
    print_malloc_2_metrics();
    void *null_re = srealloc(NULL, 16);
    print_malloc_2_metrics();
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
