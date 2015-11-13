#include <libprecisegc/libprecisegc.h>

class A {
public:
	A(int size = 1000) {
		// IS NOT GC SAFE POINT    iff object creates in heap; 		GC_NEW NESTING > 0!
		// BIT IT IS GC SAFE POINT iff object creates in the stack; GC_NEW NESTING == 0!
		a = gc_new<int>(size);
		for	(int i = 0; i < size; i++) {
			a[i] = 0;
		}
	}

private:
	gc_ptr<int> a;
};

void test (void) {
	gc_ptr<A> a = gc_new<A>(1000); // GC SAFE POINT; GC_NEW NESTING == 0
}

int main (void) {
	test();
	A a; // GC SAFE POINT; GC_NEW NESTING == 0
}