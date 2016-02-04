#include <gtest/gtest.h>

#include "libprecisegc/object.h"
#include "libprecisegc/details/gc_heap.h"
#include "libprecisegc/gcmalloc.h"

using namespace precisegc::details;

static const int OBJ_SIZE = sizeof(size_t);

TEST(gc_heap_test, test_allocate)
{
    gc_heap& heap = gc_heap::instance();
    object_meta* obj_meta = heap.allocate(OBJ_SIZE, 1, nullptr);

    size_t* ptr = (size_t*) obj_meta->get_object_ptr();
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;

    ASSERT_EQ(1, obj_meta->get_count());
    ASSERT_EQ(nullptr, obj_meta->get_class_meta());
}