//
// Created by evagl on 26.03.15.
//
#include <libprecisegc/libprecisegc.h>

struct A {
    A(int v=0xfaceface) : value(v) {}
    int value;
};

struct B {
    gc_ptr<A> a;
    B() {}
    B(gc_ptr<A> aa) : a(aa) {}
};

struct big_struct {
    void* forward_pointer;
    gc_ptr<big_struct> next;
    char data[3*4096];

    big_struct() {
        forward_pointer = this;
    }
};

void pin_test() {
    gc_ptr<A> c = gc_new<A>();
    gc_ptr<big_struct> a = gc_new<big_struct>();
    gc_ptr<big_struct> b = gc_new<big_struct>();
    a.get()->next = b;
    int some = c->value; // pin
    a.setNULL();
    b.setNULL();
    gc();
    gc();
}

int main() {
    pin_test();
    gc_ptr<A> a = gc_new<A, int>(0xdeadbeef);
    gc_ptr<B> b = gc_new<B, gc_ptr<A> >(a);
    A* ptr = a.get();
    int value = ptr->value;
    gc();
    assert(ptr != a.get());
    assert(value == a->value);
    assert(b->a.get() == a.get());
    return 0;
}