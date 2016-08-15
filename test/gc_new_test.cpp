#include <gtest/gtest.h>

#include "libprecisegc/gc_new.hpp"
#include "libprecisegc/details/type_meta.hpp"
#include "libprecisegc/details/gc_mark.h"

using namespace precisegc;
using namespace precisegc::details;

TEST(gc_new_test, test_gc_new_int)
{
    gc_ptr<int> ptr = gc_new<int>();
    ASSERT_NE(nullptr, ptr.pin().get());
    *ptr = 42;
}

TEST(gc_new_test, test_gc_new_int_array)
{
    const int ARRAY_SIZE = 10;
    gc_ptr<int[]> ptr = gc_new<int[]>(ARRAY_SIZE);
    ASSERT_NE(nullptr, ptr.pin().get());
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        ptr[i] = 42;
    }
}

namespace {

class node0
{
    gc_ptr<node0> m_ptr;
};

}

TEST(gc_new_test, test_meta)
{
    gc_ptr<node0> ptr = gc_new<node0>();
    gc_pin<node0> pin = ptr.pin();
    object_meta* obj_meta = get_object_header((void*) pin.get());
    const type_meta* cls_meta = obj_meta->get_type_meta();

    ASSERT_EQ(1, obj_meta->object_count());

    ASSERT_NE(nullptr, cls_meta);
    ASSERT_EQ(sizeof(node0), cls_meta->type_size());
    ASSERT_EQ(1, cls_meta->offsets().size());
    ASSERT_EQ(0, cls_meta->offsets()[0]);
}

namespace {

class node1
{
public:
    node1()
    {
        if (depth < MAX_DEPTH) {
            ++depth;
            m_ptr_left = gc_new<node1>();
            m_ptr_right = gc_new<node1>();
        }
    }

private:
    static const int MAX_DEPTH = 2;
    static int depth;

    gc_ptr<node1> m_ptr_left;
    gc_ptr<node1> m_ptr_right;
};

int node1::depth = 0;

}

TEST(gc_new_test, test_nested_1)
{
    gc_ptr<node1> ptr = gc_new<node1>();
    object_meta* obj_meta = get_object_header((void*) ptr.pin().get());
    const type_meta* cls_meta = obj_meta->get_type_meta();

    ASSERT_NE(nullptr, cls_meta);
    ASSERT_EQ(sizeof(node1), cls_meta->type_size());
    ASSERT_EQ(2, cls_meta->offsets().size());
    ASSERT_EQ(0, cls_meta->offsets()[0]);
    ASSERT_EQ(sizeof(gc_ptr<node1>), cls_meta->offsets()[1]);
}

namespace {

class simple_object {};

class complex_object
{
public:
    complex_object()
        : m_ptr_2(gc_new<simple_object>())
    {}
private:
    gc_ptr<simple_object> m_ptr_1;
    gc_ptr<simple_object> m_ptr_2;
    gc_ptr<simple_object> m_ptr_3;
};

}

TEST(gc_new_test, test_nested_2)
{
    gc_ptr<complex_object> ptr = gc_new<complex_object>();

    typedef type_meta_provider<simple_object> simple_meta_provider;
    typedef type_meta_provider<complex_object> complex_meta_provider;

    ASSERT_TRUE(simple_meta_provider::is_meta_created());
    ASSERT_TRUE(complex_meta_provider::is_meta_created());

    const type_meta* simple_obj_meta = simple_meta_provider::get_meta();
    ASSERT_EQ(sizeof(simple_object), simple_obj_meta->type_size());
    ASSERT_EQ(0, simple_obj_meta->offsets().size());
    ASSERT_TRUE(simple_obj_meta->is_plain_type());

    const type_meta* complex_obj_meta = complex_meta_provider::get_meta();
    ASSERT_EQ(sizeof(complex_object), complex_obj_meta->type_size());
    ASSERT_EQ(3, complex_obj_meta->offsets().size());
    ASSERT_EQ(0, complex_obj_meta->offsets()[0]);
    ASSERT_EQ(sizeof(gc_ptr<simple_object>), complex_obj_meta->offsets()[1]);
    ASSERT_EQ(2 * sizeof(gc_ptr<simple_object>), complex_obj_meta->offsets()[2]);
}

namespace {

class simple_object_with_ctor
{
public:
    simple_object_with_ctor()
    {
        gc_ptr<simple_object> p = gc_new<simple_object>();
    }
};

}

TEST(gc_new_test, test_with_ctor)
{
    gc_ptr<simple_object_with_ctor> ptr = gc_new<simple_object_with_ctor>();

    typedef type_meta_provider<simple_object_with_ctor> meta_provider;

    ASSERT_TRUE(meta_provider::is_meta_created());

    const type_meta* tmeta = meta_provider::get_meta();
    ASSERT_EQ(sizeof(simple_object_with_ctor), tmeta->type_size());
    ASSERT_EQ(0, tmeta->offsets().size());
    ASSERT_TRUE(tmeta->is_plain_type());
}
