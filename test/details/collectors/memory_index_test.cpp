#include <gtest/gtest.h>

#include <liballocgc/details/allocators/gc_core_allocator.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/utils/scope_guard.hpp>

#include "memory_descriptor_mock.h"

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::allocators;

TEST(memory_index_test, test_index)
{
    gc_core_allocator core_alloc;

    auto deleter = [&core_alloc] (byte* ptr) {
        core_alloc.deallocate(ptr, PAGE_SIZE);
    };
    std::unique_ptr<byte, decltype(deleter)> memory(core_alloc.allocate(PAGE_SIZE), deleter);

    memory_descriptor_mock mock;
    allocators::memory_index::index_gc_heap_memory(memory.get(), PAGE_SIZE, &mock);
    auto guard = utils::make_scope_guard([&memory] () {
        allocators::memory_index::deindex(memory.get(), PAGE_SIZE);
    });

    gc_memory_descriptor* descr = allocators::memory_index::get_descriptor(memory.get()).to_gc_descriptor();

    ASSERT_EQ(&mock, descr);
}