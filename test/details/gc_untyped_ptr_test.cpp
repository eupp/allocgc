#include <gtest/gtest.h>

#include <memory>
#include <utility>
#include <type_traits>

#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>
#include <libprecisegc/gc_type_meta_factory.hpp>
#include <libprecisegc/details/gc_hooks.hpp>

#include "serial_gc_mock.hpp"

using namespace precisegc;
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
    size_t val;
    byte* ptr = (byte*) &val;
    gc_untyped_ptr ptr1(ptr);
    EXPECT_EQ(ptr, ptr1.get());
}

TEST(gc_untyped_ptr_test, test_copy_construct)
{
    size_t val;
    byte* ptr = (byte*) &val;
    gc_untyped_ptr ptr1(ptr);

    gc_untyped_ptr ptr2(ptr1);
    EXPECT_EQ(ptr, ptr2.get());
}

TEST(gc_untyped_ptr_test, test_move_construct)
{
    size_t val;
    byte* ptr = (byte*) &val;
    gc_untyped_ptr ptr1(ptr);

    gc_untyped_ptr ptr2(std::move(ptr1));
    EXPECT_EQ(ptr, ptr2.get());
}

TEST(gc_untyped_ptr_test, test_nullptr_assignment)
{
    size_t val;
    byte* ptr = (byte*) &val;

    gc_untyped_ptr ptr1(ptr);
    ptr1 = nullptr;
    EXPECT_EQ(nullptr, ptr1.get());
}

TEST(gc_untyped_ptr_test, test_copy_assignment)
{
    size_t val;
    byte* ptr = (byte*) &val;

    gc_untyped_ptr ptr1(ptr);
    gc_untyped_ptr ptr2;
    ptr2 = ptr1;
    EXPECT_EQ(ptr, ptr2.get());
}

TEST(gc_untyped_ptr_test, test_move_assignment)
{
    size_t val;
    byte* ptr = (byte*) &val;

    gc_untyped_ptr ptr1(ptr);
    gc_untyped_ptr ptr2;
    ptr2 = std::move(ptr1);
    EXPECT_EQ(ptr, ptr2.get());
}

namespace {
struct test_type
{
    gc_untyped_ptr p;


};

const gc_type_meta* test_type_meta = gc_type_meta_factory<test_type>::create();
}

TEST(gc_untyped_ptr_test, test_is_root_1)
{
    using namespace allocators;

    gc_untyped_ptr ptr1;
    EXPECT_TRUE(ptr1.is_root());

    gc_alloc_response rsp = gc_allocate(sizeof(test_type), 1, test_type_meta);
    test_type* obj = reinterpret_cast<test_type*>(rsp.obj_start());
    new (obj) test_type();
    rsp.commit();
    EXPECT_FALSE(obj->p.is_root());
}

TEST(gc_untyped_ptr_test, test_is_root_2)
{
    using namespace allocators;

    size_t val;
    byte* ptr = (byte*) &val;

    gc_untyped_ptr ptr1(ptr);
    gc_untyped_ptr ptr2;


    gc_alloc_response rsp = gc_allocate(sizeof(test_type), 1, test_type_meta);
    test_type* obj = reinterpret_cast<test_type*>(rsp.obj_start());
    new (obj) test_type();
    rsp.commit();

    ptr2 = ptr1;
    EXPECT_TRUE(ptr2.is_root());

    obj->p = ptr1;
    EXPECT_FALSE(obj->p.is_root());
}

TEST(gc_untyped_ptr_test, test_bool_conversion)
{
    gc_untyped_ptr ptr1;
    EXPECT_TRUE(ptr1.is_null());

    size_t val;
    byte* ptr = (byte*) &val;

    gc_untyped_ptr ptr2(ptr);
    EXPECT_FALSE(ptr2.is_null());
}

TEST(gc_untyped_ptr_test, test_swap)
{
    size_t val;
    byte* ptr = (byte*) &val;

    gc_untyped_ptr ptr1;
    gc_untyped_ptr ptr2(ptr);

    swap(ptr1, ptr2);

    EXPECT_EQ(ptr, ptr1.get());
    EXPECT_EQ(nullptr, ptr2.get());
}