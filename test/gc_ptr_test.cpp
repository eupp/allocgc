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