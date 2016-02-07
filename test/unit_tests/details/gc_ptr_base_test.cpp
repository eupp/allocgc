#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "libprecisegc/details/gc_ptr_base.h"
#include "libprecisegc/details/gc_new_stack.h"

using namespace precisegc::details;

TEST(gc_ptr_base_test, test_construct)
{
    gc_ptr_base ptr1;
    EXPECT_EQ(nullptr, ptr1.get());

    gc_ptr_base ptr2(nullptr);
    EXPECT_EQ(nullptr, ptr2.get());

    int val;
    void* ptr = (void*) &val;
    gc_ptr_base ptr3(ptr);
    EXPECT_EQ(ptr, ptr3.get());

    gc_ptr_base ptr4(ptr3);
    EXPECT_EQ(ptr, ptr4.get());

    gc_ptr_base ptr5(std::move(ptr3));
    EXPECT_EQ(ptr, ptr5.get());
}

TEST(gc_ptr_base_test, test_is_root)
{
    gc_ptr_base ptr1;
    EXPECT_TRUE(ptr1.is_root());

    int val;
    void* ptr = (void*) &val;

    gc_new_stack::activation_entry activation_entry;
    gc_ptr_base ptr2;
    EXPECT_FALSE(ptr2.is_root());
}

TEST(gc_ptr_base_test, test_assignment)
{
    int val;
    void* ptr = (void*) &val;

    gc_ptr_base ptr1(ptr);

    gc_ptr_base ptr2;
    gc_new_stack::activation_entry activation_entry;
    gc_ptr_base ptr3;

    ptr2 = ptr1;
    EXPECT_EQ(ptr, ptr2.get());
    EXPECT_TRUE(ptr2.is_root());

    ptr3 = ptr1;
    EXPECT_EQ(ptr, ptr3.get());
    EXPECT_FALSE(ptr3.is_root());

    gc_ptr_base ptr4;
    ptr4 = std::move(ptr1);
    EXPECT_EQ(ptr, ptr4.get());

    gc_ptr_base ptr5(ptr);
    ptr5 = nullptr;
    EXPECT_EQ(nullptr, ptr5.get());
}

TEST(gc_ptr_base_test, test_bool_conversion)
{
    gc_ptr_base ptr1;
    EXPECT_FALSE(ptr1);

    int val;
    void* ptr = (void*) &val;

    gc_ptr_base ptr2(ptr);
    EXPECT_TRUE((bool) ptr2);
}

TEST(gc_ptr_base_test, test_swap)
{
    int val;
    void* ptr = (void*) &val;

    gc_ptr_base ptr1;
    gc_ptr_base ptr2(ptr);

    swap(ptr1, ptr2);

    EXPECT_EQ(ptr, ptr1.get());
    EXPECT_EQ(nullptr, ptr2.get());
}