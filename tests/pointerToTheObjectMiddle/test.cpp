#include <iostream>
#include <libprecisegc/libprecisegc.h>

using namespace std;

// gc_ptr<int> m;// = gc_new<int>();

gc_ptr<int> func ( void ) {
	cout << "1" << endl;
	gc_ptr<int> mas = gc_new<int>(10000);
	// mark_and_sweep();
	cout << "2" << endl;
	gc_ptr<int> m;
	cout << "3" << endl;
	m.attach<int>(&mas[5], mas);
	m.print();
	cout << "4" << endl;
	// cout << (void*)mas << endl;
	// m.setNULL();
	// mark_and_sweep();
	return m;
}

int main ( void ) {
	int q;
	mark_and_sweep();
	gc_ptr<int> i = func();//; i = func();
	cout << (void *) &q << " " << &i << endl;
	i.print();
	// func();
	mark_and_sweep();


	return 0;
}


