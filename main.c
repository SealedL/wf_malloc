#include <stdio.h>
#include <unistd.h>

extern void test_simple_allocate_and_free();

extern void test_allocate_after_free();

extern void test_the_worst_fit_work();

int main() {
    printf("The page size is: %d", getpagesize());
    test_simple_allocate_and_free();
    test_allocate_after_free();
    test_the_worst_fit_work();
    return 0;
}
