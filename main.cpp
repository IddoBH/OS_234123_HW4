#include <iostream>
#include "malloc_1.h"

void test_malloc_1();

int main() {
    test_malloc_1();
    return 0;
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
