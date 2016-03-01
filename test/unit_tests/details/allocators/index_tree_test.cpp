#include <gtest/gtest.h>

#include <random>
#include <limits>

#include "libprecisegc/details/allocators/index_tree.h"
#include "libprecisegc/details/allocators/debug_layer.h"
#include "libprecisegc/details/allocators/paged_allocator.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

struct index_tree_test : public ::testing::Test
{
    index_tree_test()
    {
        std::random_device r;
        std::default_random_engine e1(r());
        std::uniform_int_distribution<std::uintptr_t> uniform_dist(0, std::numeric_limits<std::uintptr_t>::max());
        for (auto& ptr: m_ptrs) {
            ptr = (uniform_dist(e1) / PAGE_SIZE) * PAGE_SIZE;
        }
    }

    typedef debug_layer<paged_allocator> allocator_t;
    typedef index_tree<int, allocator_t> tree_t;

    static const size_t SIZE = 2;
    std::uintptr_t m_ptrs[SIZE];
    tree_t m_tree;
};

TEST_F(index_tree_test, test_index_1)
{
    byte* mem = (byte*) m_ptrs[0];
    int expected = 0;
    m_tree.index(mem, PAGE_SIZE, &expected);

    ASSERT_EQ(&expected, m_tree.get_entry(mem));
}

TEST_F(index_tree_test, test_index_2)
{
    byte* mem = (byte*) m_ptrs[0];
    int expected = 0;
    m_tree.index(mem, PAGE_SIZE, &expected);

    byte* mem_end = mem + PAGE_SIZE;
    for (byte* it = mem; it < mem_end; ++it) {
        ASSERT_EQ(&expected, m_tree.get_entry(it));
    }
}

TEST_F(index_tree_test, test_remove_index)
{
    byte* mem1 = (byte*) m_ptrs[0];
    byte* mem2 = (byte*) m_ptrs[1];

    int val1 = 0;
    int val2 = 0;
    m_tree.index(mem1, PAGE_SIZE, &val1);
    m_tree.index(mem2, PAGE_SIZE, &val2);

    m_tree.remove_index(mem1, PAGE_SIZE);

    ASSERT_EQ(nullptr, m_tree.get_entry(mem1));
    ASSERT_EQ(&val2, m_tree.get_entry(mem2));
}

TEST_F(index_tree_test, test_mem_1)
{
    byte* mem = (byte*) m_ptrs[0];
    int expected = 0;
    m_tree.index(mem, PAGE_SIZE, &expected);
    m_tree.remove_index(mem, PAGE_SIZE);

    ASSERT_EQ(0, m_tree.get_const_allocator().get_allocated_mem_size());
}

TEST_F(index_tree_test, test_mem_2)
{
    {
        tree_t tree;

        byte* mem1 = (byte*) m_ptrs[0];
        byte* mem2 = (byte*) m_ptrs[1];

        int val1 = 0;
        int val2 = 0;
        tree.index(mem1, PAGE_SIZE, &val1);
        tree.index(mem2, PAGE_SIZE, &val2);
    }

    ASSERT_EQ(0, m_tree.get_const_allocator().get_allocated_mem_size());
}