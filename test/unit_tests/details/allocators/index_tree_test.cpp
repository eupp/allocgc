#include <gtest/gtest.h>

#include <random>
#include <limits>

#include "libprecisegc/details/allocators/index_tree.h"
#include "libprecisegc/details/allocators/native.h"

using namespace precisegc::details::allocators;

struct index_tree_test: public ::testing::Test
{
    index_tree_test()
    {
        std::random_device r;
        std::default_random_engine e1(r());
        std::uniform_int_distribution<std::uintptr_t> uniform_dist(0, std::numeric_limits<std::uintptr_t>::max());
        for (auto& ptr: ptrs) {
            ptr = uniform_dist(e1);
        }
    }

    static const size_t SIZE = 2;
    std::uintptr_t ptrs[SIZE];
    int entries[SIZE];
    index_tree<int, native_allocator> tree;
};

TEST_F(index_tree_test, test_index)
{
    byte* mem = (byte*) ptrs[0];
    int* expected = &entries[0];
    tree.index(mem, 0, expected);
    ASSERT_EQ(expected, tree.get_entry(mem));
}

TEST_F(index_tree_test, test_remove_index)
{
    byte* mem1 = (byte*) ptrs[0];
    byte* mem2 = (byte*) ptrs[1];
    tree.index(mem1, 0, & entries[0]);
    tree.index(mem2, 0, & entries[1]);

    tree.remove_index(mem1, 0);

    ASSERT_EQ(nullptr, tree.get_entry(mem1));
    ASSERT_EQ(&entries[1], tree.get_entry(mem2));
}