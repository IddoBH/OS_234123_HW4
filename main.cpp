#include <iostream>
//#include "malloc_1.h"
#include "malloc_2.h"

void test_malloc_1();

int main() {
//    test_malloc_1();
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

    sfree(five);
    void *five_again;
    five_again = smalloc(5);
    std::cout << five_again << std::endl;
    void* ten;
    ten = smalloc(10);
    std::cout << ten << std::endl;

    sfree(six);
    void *seven;
    seven = smalloc(7);
    std::cout << seven << std::endl;
    void *three;
    three = smalloc(3);
    std::cout << three << std::endl;

    sfree(five_again);
    sfree(three);
    three = smalloc(5);
    std::cout << three << std::endl;

    return 0;
}

//void test_malloc_1() {
//    void *zero;
//    zero = smalloc(0);
//    std::cout << zero << std::endl;
//    void *one;
//    one = smalloc(1);
//    std::cout << one << std::endl;
//    void *too_big;
//    too_big = smalloc(100000001);
//    std::cout << too_big << std::endl;
//    void *five;
//    five = smalloc(5);
//    std::cout << five << std::endl;
//    void *max;
//    max = smalloc(100000000);
//    std::cout << max << std::endl;
//    void *six;
//    six = smalloc(6);
//    std::cout << six << std::endl;
//}
