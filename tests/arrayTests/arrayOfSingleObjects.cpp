#include <libprecisegc/libprecisegc.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;

int main (void) {
  gc_ptr<int> pt = gc_new<int>(10000);
  for(int i = 0; i < 10000; i++) {
    pt[i] = i;
  }
  
  mark_and_sweep();
  
  for(int i = 0; i < 10000; i++) {
    cout << pt[i] << " ";
  }
  cout << "\n";
  return 0;
}