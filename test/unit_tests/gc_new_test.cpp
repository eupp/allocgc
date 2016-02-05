#include <gtest/gtest.h>

#include "libprecisegc/gc_new.h"
#include "libprecisegc/gcmalloc.h"
#include "libprecisegc/details/class_meta.h"

using namespace precisegc;
using namespace precisegc::details;

TEST(gc_new_test, test_gc_new_int)
{
    gc_ptr<int> ptr = gc_new<int>(1);
    ASSERT_NE(nullptr, ptr.get());
    *ptr = 42;
}

TEST(gc_new_test, test_gc_new_int_array)
{
    const int ARRAY_SIZE = 10;
    gc_ptr<int> ptr = gc_new<int>(ARRAY_SIZE);
    ASSERT_NE(nullptr, ptr.get());
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
    gc_ptr<node0> ptr = gc_new<node0>(1);
    object_meta* obj_meta = get_object_header((void*) ptr.get());
    const class_meta* cls_meta = obj_meta->get_class_meta();

    ASSERT_EQ((void*) ptr.get(), obj_meta->get_object_ptr());
    ASSERT_EQ(1, obj_meta->get_count());

    ASSERT_NE(nullptr, cls_meta);
    ASSERT_EQ(sizeof(node0), cls_meta->get_type_size());
    ASSERT_EQ(1, cls_meta->get_offsets().size());
    ASSERT_EQ(0, cls_meta->get_offsets()[0]);
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
    static const int MAX_DEPTH = 8;
    static int depth;

    gc_ptr<node1> m_ptr_left;
    gc_ptr<node1> m_ptr_right;
};

int node1::depth = 0;

}

TEST(gc_new_test, test_nested_1)
{
    gc_ptr<node1> ptr = gc_new<node1>();
    object_meta* obj_meta = get_object_header((void*) ptr.get());
    const class_meta* cls_meta = obj_meta->get_class_meta();

    ASSERT_NE(nullptr, cls_meta);
    ASSERT_EQ(sizeof(node1), cls_meta->get_type_size());
    ASSERT_EQ(2, cls_meta->get_offsets().size());
    ASSERT_EQ(0, cls_meta->get_offsets()[0]);
    ASSERT_EQ(sizeof(gc_ptr<node1>), cls_meta->get_offsets()[1]);
}

namespace {

class simple_object {};

class complex_object
{
public:
    complex_object()
        : m_ptr_2(gc_new<simple_object>(1))
    {}
private:
    gc_ptr<simple_object> m_ptr_1;
    gc_ptr<simple_object> m_ptr_2;
    gc_ptr<simple_object> m_ptr_3;
};

}

TEST(gc_new_test, test_nested_2)
{
    gc_ptr<complex_object> ptr = gc_new<complex_object>(1);

    typedef class_meta_provider<simple_object> simple_meta_provider;
    typedef class_meta_provider<complex_object> complex_meta_provider;

    ASSERT_TRUE(simple_meta_provider::is_created());
    ASSERT_TRUE(complex_meta_provider::is_created());

    const class_meta& simple_obj_meta = simple_meta_provider::get_meta();
    ASSERT_EQ(sizeof(simple_object), simple_obj_meta.get_type_size());
    ASSERT_EQ(0, simple_obj_meta.get_offsets().size());

    const class_meta& complex_obj_meta = complex_meta_provider::get_meta();
    ASSERT_EQ(sizeof(complex_object), complex_obj_meta.get_type_size());
    ASSERT_EQ(3, complex_obj_meta.get_offsets().size());
    ASSERT_EQ(0, complex_obj_meta.get_offsets()[0]);
    ASSERT_EQ(sizeof(gc_ptr<simple_object>), complex_obj_meta.get_offsets()[1]);
    ASSERT_EQ(2 * sizeof(gc_ptr<simple_object>), complex_obj_meta.get_offsets()[2]);
}