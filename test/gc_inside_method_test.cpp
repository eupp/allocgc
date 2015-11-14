#include <gtest/gtest.h>

#include <libprecisegc/libprecisegc.h>

class B;

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

    int get_value() {
        return value;
    }

    void test(gc_ptr<A> parent) {
        parent->b.setNULL();
        gc();
        gc_new<A>();
        gc_new<A>();
        gc_new<A>();
    }
};

TEST(gc_inside_method_test, test)
{
    gc_ptr<A> a = gc_new<A>();
    int v1 = a->b->get_value();
    a->b->test(a);
    int v2 = a->b->get_value();
    EXPECT_EQ(v1, v2);
}

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

    int get_value() {
        return dvalue;
    }
};

TEST(gc_inside_method_test, test_multiple_inheritance)
{
    gc_ptr<CDHolder> holder = gc_new<CDHolder>();
    int v1 = holder->cd->get_value();
    holder->cd->test(holder);
    int v2 = holder->cd->get_value();
    EXPECT_EQ(v1, v2);
}

