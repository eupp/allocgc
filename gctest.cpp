#include "heap.h"
#include <stdio.h>

struct A {
	int i;
	double k;
	double mas[100];
};

struct B {
	double k;
	double mas[120];
};

void single_test (void) {
	for (int i = 0; i < 1024 * 100; i++) {
		A * a = (A *)gcmalloc(sizeof(A), (void *)1, 1);
		A * a1 = (A *)gcmalloc(sizeof(A), (void *)1, 1);
		printf("main: a = %p sizeof(A) = %zu\n", a, sizeof(A));
		printf("main: a1 = %p sizeof(A) = %zu\n", a1, sizeof(A));
		printf("main diff a1-a: %zu i = %i\n", (size_t)a1 - (size_t)a, i);
	}
}

void array_test (int array_size) {
	for (int i = 0; i < 4 * 1024; i++) {
		A * a = (A *)gcmalloc(sizeof(A), (void *)1, array_size);
		A * a1 = (A *)gcmalloc(sizeof(A), (void *)1, array_size);
		printf("main: a = %p sizeof(A) = %zu\n", a, sizeof(A));
		printf("main diff a1-a: %zu i = %i\n", (size_t)a1 - (size_t)a, i); fflush(stdout);
	}
}

void mask_test (void) {
	A * a = (A *)gcmalloc(sizeof(A), (void *)1, 1);
	A * a1 = (A *)gcmalloc(sizeof(A), (void *)1, 1);
}

int main (void) {
	init_segregated_storage();
	// single_test();
	// array_test(100);
	mask_test();

	return 0;
}

