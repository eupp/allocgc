#include <gtest/gtest.h>

#include <liballocgc/liballocgc.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/gc_type_meta.hpp>

using namespace allocgc;
using namespace allocgc::serial;
using namespace allocgc::details;
using namespace allocgc::details::collectors;

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
    gc_cell cell = allocators::memory_index::get_gc_cell((byte*) pin.get());
    const gc_type_meta* type_meta = cell.get_type_meta();

    ASSERT_EQ(1, cell.object_count());

    ASSERT_NE(nullptr, type_meta);
    ASSERT_EQ(sizeof(node0), type_meta->type_size());
    ASSERT_EQ(1, type_meta->offsets().size());
    ASSERT_EQ(0, type_meta->offsets()[0]);
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
    gc_cell cell = allocators::memory_index::get_gc_cell((byte*) ptr.pin().get());
    const gc_type_meta* type_meta = cell.get_type_meta();

    ASSERT_NE(nullptr, type_meta);
    ASSERT_EQ(sizeof(node1), type_meta->type_size());
    ASSERT_EQ(2, type_meta->offsets().size());
    ASSERT_EQ(0, type_meta->offsets()[0]);
    ASSERT_EQ(sizeof(gc_ptr<node1>), type_meta->offsets()[1]);
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

    typedef gc_type_meta_factory<simple_object> simple_meta_factory;
    typedef gc_type_meta_factory<complex_object> complex_meta_factory;

    ASSERT_NE(nullptr, simple_meta_factory::get());
    ASSERT_NE(nullptr, complex_meta_factory::get());

    const gc_type_meta* simple_obj_meta = simple_meta_factory::get();
    ASSERT_EQ(sizeof(simple_object), simple_obj_meta->type_size());
    ASSERT_EQ(0, simple_obj_meta->offsets().size());
    ASSERT_TRUE(simple_obj_meta->is_plain_type());

    const gc_type_meta* complex_obj_meta = complex_meta_factory::get();
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
//        check_rootness(p);
    }

    static void check_rootness(const gc_ptr<simple_object>& p)
    {
        ASSERT_TRUE(p.is_root());
    }
};

}

TEST(gc_new_test, test_with_ctor)
{
    gc_ptr<simple_object_with_ctor> ptr = gc_new<simple_object_with_ctor>();

    typedef gc_type_meta_factory<simple_object_with_ctor> meta_factory;

    ASSERT_NE(nullptr, meta_factory::get());

    const gc_type_meta* tmeta = meta_factory::get();
    ASSERT_EQ(sizeof(simple_object_with_ctor), tmeta->type_size());
    ASSERT_EQ(0, tmeta->offsets().size());
    ASSERT_TRUE(tmeta->is_plain_type());
}