#include <gtest/gtest.h>

#include <unordered_set>

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/ptrs/trace_ptr.hpp>
#include <libprecisegc/details/gc_heap.hpp>

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::ptrs;

namespace {
struct simple_object {};

struct complex_object
{
    gc_untyped_ptr m_ptr1;
    gc_untyped_ptr m_ptr2;
};

static const size_t OBJ_SIZE = sizeof(complex_object) + sizeof(object_meta);
}

class trace_ptr_test : public ::testing::Test
{
public:
    trace_ptr_test()
        : heap(gc_compacting::DISABLED)
    {
        type_meta_provider<simple_object>::create_meta();
        type_meta_provider<complex_object>::create_meta(std::vector<size_t>({0, sizeof(gc_untyped_ptr)}));

        simple_object_ptr = heap.allocate(OBJ_SIZE);
        new (simple_object_ptr.get_meta()) object_meta(type_meta_provider<simple_object>::get_meta());
        simple_object_ptr.get_meta()->set_object_count(1);

        complex_object_ptr = heap.allocate(OBJ_SIZE);
        new (complex_object_ptr.get_meta()) object_meta(type_meta_provider<complex_object>::get_meta());
        complex_object_ptr.get_meta()->set_object_count(1);

        child1 = heap.allocate(OBJ_SIZE).get();
        child2 = heap.allocate(OBJ_SIZE).get();

        complex_object* p = reinterpret_cast<complex_object*>(complex_object_ptr.get());
        p->m_ptr1 = gc_untyped_ptr(child1);
        p->m_ptr2 = gc_untyped_ptr(child2);
    }

    managed_ptr simple_object_ptr;
    managed_ptr complex_object_ptr;
    byte* child1;
    byte* child2;
    gc_heap heap;
};

TEST_F(trace_ptr_test, test_simple_object)
{
    std::unordered_set<byte*> empty_set;
    std::unordered_set<byte*> traced_set;
    trace_ptr(simple_object_ptr, [&traced_set] (managed_ptr p) {
        traced_set.insert(p.get());
    });

    ASSERT_EQ(empty_set, traced_set);
}

TEST_F(trace_ptr_test, test_complex_object)
{
    std::unordered_set<byte*> children({child1, child2});
    std::unordered_set<byte*> traced_set;
    trace_ptr(complex_object_ptr, [&traced_set] (managed_ptr p) {
        traced_set.insert(p.get());
    });

    ASSERT_EQ(children, traced_set);
}