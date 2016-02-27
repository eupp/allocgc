#include <gtest/gtest.h>

#include <random>
#include <limits>

#include "libprecisegc/details/allocators/index_tree.h"
#include "libprecisegc/details/allocators/native.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

struct index_tree_test: public ::testing::Test
{
    index_tree_test()
    {
        std::random_device r;
        std::default_random_engine e1(r());
        std::uniform_int_distribution<std::uintptr_t> uniform_dist(0, std::numeric_limits<std::uintptr_t>::max());
        for (auto& ptr: ptrs) {
            ptr = (uniform_dist(e1) / PAGE_SIZE) * PAGE_SIZE;
        }
    }

    static const size_t SIZE = 2;
    std::uintptr_t ptrs[SIZE];
    int entries[SIZE];
    index_tree<int, native_allocator> tree;
};

TEST_F(index_tree_test, test_index_1)
{
    byte* mem = (byte*) ptrs[0];
    int* expected = &entries[0];
    tree.index(mem, PAGE_SIZE, expected);

    int* t = tree.get_entry(mem);
//    ASSERT_EQ(expected, );
}

TEST_F(index_tree_test, test_index_2)
{
    byte* mem = (byte*) ptrs[0];
    int* expected = &entries[0];
    tree.index(mem, PAGE_SIZE, expected);

    byte* mem_end = mem + PAGE_SIZE;
    for (byte* it = mem; it < mem_end; ++it) {
        ASSERT_EQ(expected, tree.get_entry(it));
    }
}

TEST_F(index_tree_test, test_remove_index)
{
    byte* mem1 = (byte*) ptrs[0];
    byte* mem2 = (byte*) ptrs[1];
    tree.index(mem1, PAGE_SIZE, &entries[0]);
    tree.index(mem2, PAGE_SIZE, &entries[1]);

    tree.remove_index(mem1, PAGE_SIZE);

    ASSERT_EQ(nullptr, tree.get_entry(mem1));
    ASSERT_EQ(&entries[1], tree.get_entry(mem2));
}