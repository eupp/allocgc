#include <libprecisegc/libprecisegc.h>

class B; // forward declaration

class A {
public:
    gc_ptr<B> b;
    A() {
        b = gc_new<B, int>(0x0000face);
    }
};

class B {
private:
    int value;
public:
    B(int v = 0xdeadbeef) : value(v) {}

    void test(gc_ptr<A> parent) {
        int v = value;
        parent->b.setNULL();
        gc();
        gc_new<A>();
        gc_new<A>();
        gc_new<A>();
        assert(v == value);
    }
};

class C {
public:
    int value;
    C(int v) : value(v) {};
};


class CD;

class CDHolder {
public:
    gc_ptr<CD> cd;
    CDHolder() {
        cd = gc_new<CD, int>(0x0000face);
    }
};

class D{
public:
    int dvalue;
    D(int v = 0xdeadbeef) : dvalue(v) {}

    void test(gc_ptr<CDHolder> holder) {
        int v = dvalue;
        holder->cd.setNULL();
        gc();
        gc_new<CD>();
        gc_new<CD>();
        gc_new<CD>();
        assert(v == dvalue);
    }
};


class CD : public C, public D {
public:
    CD(int v = 0xaaaaffff) : D(v), C(v) {}
};

int main() {
    C c(100);
    std::cout << c.value << std::endl; // to be used

    gc_ptr<A> a = gc_new<A>();
    a->b->test(a);

    gc_ptr<CDHolder> holder = gc_new<CDHolder>();
    holder->cd->test(holder);
    return 0;
}
