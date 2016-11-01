#include <gtest/gtest.h>

#include <unordered_set>

#include <libprecisegc/details/gc_type_meta_factory.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>
#include <libprecisegc/details/collectors/trace_ptr.hpp>
#include <libprecisegc/details/gc_heap.hpp>

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::ptrs;
using namespace precisegc::details::collectors;

namespace {
struct simple_object_t {};

struct complex_object_t
{
    gc_untyped_ptr m_ptr1;
    gc_untyped_ptr m_ptr2;
};

static const size_t OBJ_SIZE = sizeof(complex_object_t) + sizeof(traceable_object_meta);
}

class trace_ptr_test : public ::testing::Test
{
public:
    trace_ptr_test()
        : heap(gc_compacting::DISABLED)
    {
        gc_type_meta_factory<simple_object_t>::create();
        gc_type_meta_factory<complex_object_t>::create(std::vector<size_t>({0, sizeof(gc_untyped_ptr)}));

        gc_alloc_request rqst_simple(OBJ_SIZE, 1, gc_type_meta_factory<simple_object_t>::get());
        gc_alloc_response alloc_dscr1 = heap.allocate(rqst_simple);

        gc_alloc_request rqst_complex(OBJ_SIZE, 1, gc_type_meta_factory<complex_object_t>::get());
        gc_alloc_response alloc_dscr2 = heap.allocate(rqst_complex);

        gc_alloc_request rqst_child(OBJ_SIZE, 1, nullptr);
        child1 = managed_object::get_object(heap.allocate(rqst_child).get());
        child2 = managed_object::get_object(heap.allocate(rqst_child).get());

        simple_object = managed_object::make(alloc_dscr1.get());
        complex_object = managed_object::make(alloc_dscr2.get());

        complex_object_t* p = reinterpret_cast<complex_object_t*>(complex_object.object());
        p->m_ptr1 = gc_untyped_ptr(child1);
        p->m_ptr2 = gc_untyped_ptr(child2);
    }

    managed_object simple_object;
    managed_object complex_object;
    byte* child1;
    byte* child2;
    gc_heap heap;
};

TEST_F(trace_ptr_test, test_simple_object)
{
    std::unordered_set<byte*> empty_set;
    std::unordered_set<byte*> traced_set;
    trace_ptr(simple_object, [&traced_set] (byte* ptr) {
        traced_set.insert(ptr);
    });

    ASSERT_EQ(empty_set, traced_set);
}

TEST_F(trace_ptr_test, test_complex_object)
{
    std::unordered_set<byte*> children({child1, child2});
    std::unordered_set<byte*> traced_set;
    trace_ptr(complex_object, [&traced_set] (byte* ptr) {
        traced_set.insert(ptr);
    });

    ASSERT_EQ(children, traced_set);
}