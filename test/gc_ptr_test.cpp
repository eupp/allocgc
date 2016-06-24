#include <gtest/gtest.h>

#include "libprecisegc/gc_ptr.h"
#include "libprecisegc/gc_new.h"

using namespace precisegc;

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