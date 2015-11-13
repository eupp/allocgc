#include <libprecisegc/libprecisegc.h>
#include <math.h>

const int n = 3;

void produce_garbage (void) {
	gc_ptr<int> array;
	for (int i = 0; i < n; i++) {
		array = gc_new<int>(pow(2, i) * 100);
	}
}

class A {
public:
	int a;
	double b;
	size_t c;
	size_t c1;
	size_t c2;
	size_t c3;
	size_t cw;
	size_t ce;
	size_t cr;
	size_t ct;
	size_t cy;
	size_t cu;
	size_t ci;
	size_t co;
	size_t cp;
	size_t ca;

	A() {}
};

int main (void) {
	// gc_ptr<int> array; // = gc_new<int>(10);
	// printf("%p %p \n", array.get(), &array);
	produce_garbage();
	gc_ptr<A> a = gc_new<A>();
	// // force gc call!
	// printf("force gc call\n");
	gc();
	return 0;
}