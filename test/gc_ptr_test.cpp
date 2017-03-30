#include <gtest/gtest.h>

#include <liballocgc/gc_cast.hpp>

#include "liballocgc/gc_ptr.hpp"
#include "liballocgc/gc_new.hpp"

using namespace allocgc;

TEST(gc_ptr_test, test_gc_ptr)
{
    static const size_t SIZE = 8;
    gc_ptr<int[]> ptr1 = gc_new<int[]>(SIZE);

    gc_ptr<int[]> ptr2 = ptr1;
    ASSERT_EQ(ptr1, ptr2);

    ++ptr2;
    ASSERT_NE(ptr1, ptr2);

    --ptr2;
    ASSERT_EQ(ptr1, ptr2);

    ptr2 = ptr1 + SIZE;

    gc_pin<int[]> pin1 = ptr1.pin();
    gc_pin<int[]> pin2 = ptr2.pin();
    ASSERT_EQ(pin2.get(), pin1.get() + SIZE);

    ptr2 -= SIZE;
    ASSERT_EQ(ptr1, ptr2);
}

namespace {

struct Base1 {
    int b1;
};

struct Base2 {
    short b2;
};

struct Derived : public Base1, public Base2 {
    double d;
};

}

TEST(gc_ptr_test, test_upcast_constructor)
{
    gc_ptr<Derived> derived = gc_new<Derived>();
    derived->b1 = 42;
    derived->b2 = 11;
    derived->d  = 3.14;

    gc_ptr<Base1> base1 = derived;
    ASSERT_EQ(42, base1->b1);
    base1->b1 = 0;

    gc_ptr<Base2> base2 = derived;
    ASSERT_EQ(11, base2->b2);
    base2->b2 = 0;

    ASSERT_EQ(0, derived->b1);
    ASSERT_EQ(0, derived->b2);
    ASSERT_DOUBLE_EQ(3.14, derived->d);
}

namespace {

struct A {
    size_t d1;
    size_t d2;
};

}

TEST(gc_ptr_test, test_take)
{
    gc_ptr<A> p = gc_new<A>();
    p->d1 = -1;
    p->d2 = 42;

    gc_ptr<size_t> pInner = take_interior<A, size_t, &A::d2>(p);
    ASSERT_EQ(42, *pInner);
}