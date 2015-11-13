#include <libprecisegc/libprecisegc.h>

const int array_size = 100;

class A {
	gc_ptr<int> a1;
	gc_ptr<int> a2;
	gc_ptr<int> a3;
	gc_ptr<int> a4;

public:
	A () {
		a1 = gc_new<int>(array_size);
		a2 = gc_new<int>(array_size);
		a3 = gc_new<int>(array_size);
		a4 = gc_new<int>(array_size);
	}
};

class B {
	gc_ptr<A> a1;
	gc_ptr<A> a2;
	gc_ptr<A> a3;
	gc_ptr<A> a4;

public:
	B () {
		a1 = gc_new<A>();
		a2 = gc_new<A>();
		a3 = gc_new<A>();
		a4 = gc_new<A>();
	}
};

class C {
	gc_ptr<int> a1;
	gc_ptr<int> a2;

public:
	C () {
		a1 = gc_new<int>(100);
		// a2 = gc_new<int>(100);
	}
};

template <typename T>
void FreeAll( T & t ) {
    T tmp;
    t.swap( tmp );
}

void test () {
	// gc_ptr<A> a = gc_new<A>();
	gc_ptr<B> b = gc_new<B>();
	// gc_ptr<C> c = gc_new<C>();
	// gc_ptr<A> a = gc_new<A>();
	// void * a = malloc (400);
	// void * b = malloc (400);
	// void * c = malloc (400);
	// transfer_to_automatic_objects(a);
	// transfer_to_automatic_objects(b);
	// transfer_to_automatic_objects(c);
	// std::vector<size_t> v;
	// for (int i = 0; i < 10; i++) {
	// 	v.push_back(i);
	// }
	// std::vector<size_t>().swap(v);
	// v.shrink_to_fit();
	gc();
}

int main (void) {
	// gc();
	// gc_ptr<B> b = gc_new<B>();
	// gc_ptr<A> a = gc_new<A>();
	test();
	gc();
	// test();
	// gc();
	return 0;
}
