#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/ptrs/gc_new_stack.hpp>
#include <libprecisegc/details/gc_hooks.hpp>

#include "serial_gc_mock.hpp"

using namespace precisegc::details;
using namespace precisegc::details::ptrs;

using ::testing::_;
using ::testing::Exactly;

TEST(gc_untyped_ptr_test, test_default_construct)
{
    gc_untyped_ptr ptr;
    EXPECT_EQ(nullptr, ptr.get());
}

TEST(gc_untyped_ptr_test, test_nullptr_construct)
{
    gc_untyped_ptr ptr(nullptr);
    EXPECT_EQ(nullptr, ptr.get());
}

TEST(gc_untyped_ptr_test, test_raw_ptr_construct)
{
    byte val;
    byte* ptr = &val;
    gc_untyped_ptr ptr1(ptr);
    EXPECT_EQ(ptr, ptr1.get());
}

TEST(gc_untyped_ptr_test, test_copy_construct)
{
    byte val;
    byte* ptr = &val;
    gc_untyped_ptr ptr1(ptr);

    gc_untyped_ptr ptr2(ptr1);
    EXPECT_EQ(ptr, ptr2.get());
}

TEST(gc_untyped_ptr_test, test_move_construct)
{
    byte val;
    byte* ptr = &val;
    gc_untyped_ptr ptr1(ptr);

    gc_untyped_ptr ptr2(std::move(ptr1));
    EXPECT_EQ(ptr, ptr2.get());
}

TEST(gc_untyped_ptr_test, test_nullptr_assignment)
{
    byte val;
    byte* ptr = &val;

    gc_untyped_ptr ptr1(ptr);
    ptr1 = nullptr;
    EXPECT_EQ(nullptr, ptr1.get());
}

TEST(gc_untyped_ptr_test, test_copy_assignment)
{
    byte val;
    byte* ptr = &val;

    gc_untyped_ptr ptr1(ptr);
    gc_untyped_ptr ptr2;
    ptr2 = ptr1;
    EXPECT_EQ(ptr, ptr2.get());
}

TEST(gc_untyped_ptr_test, test_move_assignment)
{
    byte val;
    byte* ptr = &val;

    gc_untyped_ptr ptr1(ptr);
    gc_untyped_ptr ptr2;
    ptr2 = std::move(ptr1);
    EXPECT_EQ(ptr, ptr2.get());
}

TEST(gc_untyped_ptr_test, test_is_root_1)
{
    gc_untyped_ptr ptr1;
    EXPECT_TRUE(ptr1.is_root());

    gc_new_stack::activation_entry activation_entry;
    gc_untyped_ptr ptr2;
    EXPECT_FALSE(ptr2.is_root());
}

TEST(gc_untyped_ptr_test, test_is_root_2)
{
    byte val;
    byte* ptr = &val;

    gc_untyped_ptr ptr1(ptr);

    gc_untyped_ptr ptr2;
    gc_new_stack::activation_entry activation_entry;
    gc_untyped_ptr ptr3;

    ptr2 = ptr1;
    EXPECT_TRUE(ptr2.is_root());

    ptr3 = ptr1;
    EXPECT_FALSE(ptr3.is_root());
}

TEST(gc_untyped_ptr_test, test_bool_conversion)
{
    gc_untyped_ptr ptr1;
    EXPECT_TRUE(ptr1.is_null());

    byte val;
    byte* ptr = &val;

    gc_untyped_ptr ptr2(ptr);
    EXPECT_FALSE(ptr2.is_null());
}

TEST(gc_untyped_ptr_test, test_swap)
{
    byte val;
    byte* ptr = &val;

    gc_untyped_ptr ptr1;
    gc_untyped_ptr ptr2(ptr);

    swap(ptr1, ptr2);

    EXPECT_EQ(ptr, ptr1.get());
    EXPECT_EQ(nullptr, ptr2.get());
}