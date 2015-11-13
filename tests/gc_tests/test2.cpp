#include <libprecisegc/libprecisegc.h>

const int n = 1000;

void produce_garbage (void) {
	gc_ptr<int> array;
	for (int i = 0; i <  n; i++) {
		array = gc_new<int>(i);
		for (int j = 0; j < i; j++) {
			array[j] = j;
		}
	}
}

int main (void) {
	gc_ptr<int> global_array = gc_new<int>(100000000);
	produce_garbage();
	gc();
	return 0;
}