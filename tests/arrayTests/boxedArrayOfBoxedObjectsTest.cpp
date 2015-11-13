#include <libprecisegc/libprecisegc.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;

int con  = 100;
class B {
  int i;
public:
  B() { i = con; con ++;}
  
  void print() {
    cout << i;
  }
};

class A{
  gc_ptr<B> i;
  gc_ptr<B> j;
public:
  A() { i = gc_new<B>(); j = gc_new<B>();}
  
  void print () {
    i->print();
    cout << "; ";
    j->print();
    cout << "\n";
  }
};
  
  
int main (void) {

  gc_ptr<A> pt = gc_new<A>(3);
  mark_and_sweep();
  cout << (sizeof(A) + sizeof(B) * 2) * 3 << "\n";
  for( int i = 0; i < 3; i++) {
    pt[i].print();
  }
  
  return 0;
}