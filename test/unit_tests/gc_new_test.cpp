#include <gtest/gtest.h>

#include "libprecisegc/gc_new.h"
#include "libprecisegc/gcmalloc.h"

using namespace precisegc;
using namespace precisegc::details;

TEST(gc_new_test, test_gc_new)
{
    gc_ptr<int> ptr = gc_new<int>(1);
    ASSERT_NE(nullptr, ptr.get());
    *ptr = 42;
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
            m_ptr_1 = gc_new<node1>();
            m_ptr_2 = gc_new<node1>();
        }
    }

private:
    static const int MAX_DEPTH = 8;
    static int depth;

    gc_ptr<node1> m_ptr_1;
    gc_ptr<node1> m_ptr_2;
};

int node1::depth = 0;

}

TEST(gc_new_test, test_nested)
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
