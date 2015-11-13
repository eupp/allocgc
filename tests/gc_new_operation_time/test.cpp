#include <libprecisegc/libprecisegc.h>
#include <iostream>
#include <math.h>
#include <sys/time.h>

#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

unsigned stats_rtclock (void) {
	struct timeval t;
	struct timezone tz;

	if (gettimeofday( &t, &tz ) == -1)
	return 0;
	return (unsigned)(t.tv_sec * 1000 + t.tv_usec / 1000);
}

#define array_size 6
#define ENABLE_C_COMPARE

using std::cout;
using std::endl;

class A {
	gc_ptr<int> a, b;
public:
	A() {	a = gc_new<int>(10);	b = gc_new<int>(10);	}
};
class AA {
	int * a, * b;
public:
	AA() {	a = new int[10];	b = new int[10];	}
};

class B {
	gc_ptr<A> a;
	gc_ptr<int> i;
public:
	B() {	a = gc_new<A>();	i = gc_new<int>(100);	}
};
class BB {
	AA * a;
	int * i;
public:
	BB() {	a = new AA();	i = new int[100];	}
};

class C {
	gc_ptr<A> a;
	gc_ptr<B> b;
	gc_ptr<int> i;
public:
	C() {	a = gc_new<A>(100);	b = gc_new<B>(100);	i = gc_new<int>(100);	}
};
class CC {
	AA * a;
	BB * b;
	int * i;
public:
	CC() {	a = new AA[100];	b = new BB[100];	i = new int[100];	}
};

void unboxed_array_test () {
	long tBegin, tEnd;
	cout << "gc_new test" << endl;
	cout << "arrays:\n";
	cout << "\tarrays:\n";
	int coef = 4;
	cout << "\tprimitive type:\n";
	for (int i = 0; i < array_size; i++) {
		int size = pow(coef, i);
		cout << "\t\tallocate array type int, size " << size << " took ";
		tBegin = currentTime();
		gc_ptr<int> aaa = gc_new<int>(size);
		tEnd = currentTime();
		cout << elapsedTime(tEnd - tBegin) << " msec" << endl;
	}

	#ifdef ENABLE_C_COMPARE
		cout << "\tsame in pure C:" << endl;
		tEnd = currentTime();
		for (int i = 0; i < array_size; i++) {
			int size = pow(coef, i);
			cout << "\t\tallocate array type int, size " << size << " took ";
			tBegin = currentTime();
			new int[size];
			tEnd = currentTime();
			cout << elapsedTime(tEnd - tBegin) << " msec" << endl;
		}
	#endif
}

void boxed_array_test () {
	long tBegin, tEnd;
	cout << "\tnon-primitive type:\n";
	int coef = 4;
	for (int i = 0; i < array_size; i++) {
		int size = pow(coef, i);
		cout << "\t\tallocate array type A with 2 gc_ptr<int> size 10, size " << size << " took ";
		tBegin = currentTime();
		gc_ptr<A> aaa = gc_new<A>(size);
		tEnd = currentTime();
		cout << elapsedTime(tEnd - tBegin) << " msec " << endl;
	}

	#ifdef ENABLE_C_COMPARE
		cout << "\tsame in pure C:" << endl;
		for (int i = 0; i < array_size; i++) {
			int size = pow(coef, i);
			cout << "\t\tallocate array type A with 2 int * size 10, size " << size << " took ";
			tBegin = currentTime();
			new AA[size];
			tEnd = currentTime();
			cout << elapsedTime(tEnd - tBegin) << " msec " << endl;
		}
	#endif
}

void lots_of_different_time_objects () {
	long tBegin, tEnd;
	cout << "\nlots of diff objects: ";
	tBegin = currentTime();
	for (int i = 0; i < array_size; i++) {
		gc_new<C>(100);
		gc_new<B>(100);
		gc_new<A>(100);
		gc_new<A>();
		gc_new<B>();
		gc_new<C>();
	}
	tEnd = currentTime();
	cout << elapsedTime(tEnd - tBegin) << " msec " << endl;

	#ifdef ENABLE_C_COMPARE
		cout << "same in pure C: ";
		tBegin = currentTime();
		for (int i = 0; i < array_size; i++) {
			new CC[100];
			new BB[100];
			new AA[100];
			new AA();
			new BB();
			new CC();
		}
		tEnd = currentTime();
		cout << elapsedTime(tEnd - tBegin) << " msec " << endl;
	#endif
}

int main (void) {
	unboxed_array_test();
	mark_and_sweep();
	boxed_array_test();
	mark_and_sweep();
	lots_of_different_time_objects();
	mark_and_sweep();

	return 1;
}