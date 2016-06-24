#include <libprecisegc/libprecisegc.h>
#include <stdio.h>

class A {
private:
  int size;
  gc_ptr<int> mas;
public:
  A () {}

  A (int len, const gc_ptr<int> & ar) {
    printf("A ... ");
    fflush(stdout);
    size = len;
    this->mas = gc_new<int>(len);
    printf("gc_new...ends\n");
    fflush(stdout);
    for (int i = 0; i < len; i++) {
      mas[i] = ar[i];
    }
    printf("ends\n");
    fflush(stdout);
  }

  void print () {
    for (int i = 0; i < size; i++) {
      printf("%i ", mas[i]);
    }
    printf("\n");
  }
};

int main (void) {
  int len = 10;
  gc_ptr<int> ar = gc_new<int>(len), br = gc_new<int>(len);
  for (int i = 0; i < len; i++) {
    ar[i] = i;
    br[i] = len - i;
  }

  gc_ptr<A> a = gc_new<A, int, gc_ptr<int>>(len, ar, 1);
  a->print();
  gc_ptr<A> b = gc_new<A, int, gc_ptr<int>>(len, br);
  for (int i = 0; i < len; i++) {
    ar[i] += 10;
    br[i] += 100;
  }
  a->print(); b->print();
  printf("\n");
  return 0;
}
