#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/gc_core_allocator.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
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
    std::unique_ptr<byte, decltype(deleter)> memory(gc_core_allocator::allocate(PAGE_SIZE), deleter);

    memory_descriptor_mock mock;
    memory_index::add_to_index(memory.get(), PAGE_SIZE, &mock);
    auto guard = utils::make_scope_guard([&memory] () {
        memory_index::remove_from_index(memory.get(), PAGE_SIZE);
    });

    memory_descriptor* descr = memory_index::index_memory(memory.get());

    ASSERT_EQ(&mock, descr);
}