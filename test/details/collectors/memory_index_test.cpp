#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/gc_core_allocator.hpp>
#include <libprecisegc/details/allocators/memory_index.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>

#include "memory_descriptor_mock.h"

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::collectors;
using namespace precisegc::details::allocators;

TEST(memory_index_test, test_index)
{
    auto deleter = [] (byte* ptr) {
        gc_core_allocator::deallocate(ptr, PAGE_SIZE);
    };
    std::unique_ptr<byte, decltype(deleter)> memory(gc_core_allocator::allocate(PAGE_SIZE, false), deleter);

    memory_descriptor_mock mock;
    allocators::memory_index::index_gc_heap_memory(memory.get(), PAGE_SIZE, &mock);
    auto guard = utils::make_scope_guard([&memory] () {
        allocators::memory_index::deindex(memory.get(), PAGE_SIZE);
    });

    gc_memory_descriptor* descr = allocators::memory_index::get_descriptor(memory.get()).to_gc_descriptor();

    ASSERT_EQ(&mock, descr);
}