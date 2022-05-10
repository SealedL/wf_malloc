//
// Created on 5/10/22.
//

#include <stdio.h>

#include "wf_malloc.h"

void check_errno() {
    if (errno != 0) {
        printf("error occurred!\n");
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }
}

void test_simple_allocate_and_free() {
    printf("----test_simple_allocate_and_free started:\n");
    printf("allocate array has 40 int elements.\n");

    int *array = (int *) wf_malloc(10 * 4 * sizeof(int));

    printf("array: \t\t%p\n", array);
    printf("free the array.\n");

    wf_free(array);

    printf("----test_simple_allocate_and_free ended.\n\n");
}

void test_allocate_after_free() {
    printf("----test_allocate_after_free started:\n");
    printf("allocate array has 40 int elements.\n");

    int *array = (int *) wf_malloc(10 * 4 * sizeof(int));

    printf("array1 allocated: \t\t%p\n", array);
    printf("free the array.\n");

    wf_free(array);

    printf("allocate array2 has 400 int elements.\n");

    int *second_array = (int *) wf_malloc(100 * 4 * sizeof(int));

    printf("array2 allocated: \t\t%p\n", second_array);
    printf("free the array2.\n");

    wf_free(second_array);

    printf("----test_allocate_after_free ended.\n\n");
}

void test_the_worst_fit_work() {
    printf("----test_the_worst_fit_work started:\n");
    printf("allocate array1 has 400 int elements.\n");

    int *array1 = (int *) wf_malloc(100 * 4 * sizeof(int));

    printf("array1 allocated: \t\t%p\n", array1);
    printf("allocate array2 has 3600 int elements.\n");

    int *array2 = (int *) wf_malloc(900 * 4 * sizeof(int));

    printf("array2 allocated: \t\t%p\n", array2);
    printf("free the array1.\n");

    wf_free(array1);

    printf("allocate array3 has 40 int elements.\n");

    int *array3 = (int *) wf_malloc(10 * 4 * sizeof(int));

    printf("array3 allocated: \t\t%p\n", array3);
    printf("free the array2.\n");

    wf_free(array2);

    printf("allocate array3 has 40 int elements.\n");

    int *array4 = (int *) wf_malloc(10 * 4 * sizeof(int));

    printf("array4 allocated: \t\t%p\n", array4);

    printf("free the array3.\n");

    wf_free(array3);

    printf("free the array4.\n");
    wf_free(array4);

    printf("----test_the_worst_fit_work ended.\n\n");
}
