#include <libprecisegc/libprecisegc.h>
#include <iostream>

struct str2 {
  gc_ptr<int> p;
};

struct str1 {
  gc_ptr<str2> p;
};

int main (void) {
  str1 * s = (str1 *) malloc (sizeof(str1));
  s->p = gc_new<str2>();
  register_object(&(s->p));
  s->p->p = gc_new<int>(1000);
  
  mark_and_sweep();
  
  unregister_object(&(s->p));
  
  mark_and_sweep();
  
  transfer_to_automatic_objects(s);
  
  mark_and_sweep();

  return 0;
}