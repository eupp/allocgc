#include <gtest/gtest.h>

#include <random>
#include <limits>
#include <libprecisegc/details/collectors/index_tree.hpp>

#include "libprecisegc/details/collectors/index_tree.hpp"
#include "libprecisegc/details/allocators/debug_layer.hpp"
#include "libprecisegc/details/allocators/core_allocator.hpp"

using namespace precisegc::details;
using namespace precisegc::details::collectors;

//TEST(index_tree_level_test, test_level)
//{
//    typedef internals::index_tree_access::inter_level<
//                  int
//                , internals::index_tree_access::last_level<int>
//            > level_t;
//
//    typedef internals::index_tree_access::idxs_t idxs_t;
//
//    level_t level;
//    idxs_t idxs;
//
//    ASSERT_LE(2, idxs.size());
//
//    idxs[0] = 0; idxs[1] = 0;
//    int entry = 0;
//    level.add_to_index(idxs.begin(), &entry);
//
//    ASSERT_EQ(&entry, level.get(idxs.begin()));
//    ASSERT_EQ(1, internals::index_tree_access::get_level_count<int>(level, idxs[0]));
//}
//
//TEST(index_tree_level_test, test_nested_levels)
//{
//    typedef internals::index_tree_access::internal_level<
//              int
//            , internals::index_tree_access::last_level<int>
//        > nested_level_t;
//
//    typedef internals::index_tree_access::internal_level<
//              int
//            , nested_level_t
//    > level_t;
//
//    typedef internals::index_tree_access::idxs_t idxs_t;
//
//    level_t level;
//    idxs_t idxs1;
//    idxs_t idxs2;
//
//    ASSERT_LE(3, idxs1.size());
//
//    idxs1[0] = 0; idxs1[1] = 0; idxs1[2] = 0;
//    idxs2[0] = 0; idxs2[1] = 1; idxs2[2] = 0;
//
//    int entry1 = 0;
//    int entry2 = 0;
//    level.add_to_index(idxs1.begin(), &entry1);
//    level.add_to_index(idxs2.begin(), &entry2);
//
//    ASSERT_EQ(&entry1, level.get(idxs1.begin()));
//    ASSERT_EQ(&entry2, level.get(idxs2.begin()));
//
//    ASSERT_EQ(2, internals::index_tree_access::get_level_count<int>(level, idxs1[0]));
//
//    nested_level_t* nested_level = internals::index_tree_access::get_level<int>(level, idxs1[0]);
//    ASSERT_EQ(1, internals::index_tree_access::get_level_count<int>(*nested_level, idxs1[1]));
//    ASSERT_EQ(1, internals::index_tree_access::get_level_count<int>(*nested_level, idxs2[1]));
//}

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

    typedef index_tree<int> tree_t;

    static const size_t SIZE = 2;
    std::uintptr_t m_ptrs[SIZE];
    tree_t m_tree;
};

TEST_F(index_tree_test, test_index_1)
{
    byte* mem = (byte*) m_ptrs[0];
    int entry = 0;
    m_tree.add_to_index(mem, PAGE_SIZE, &entry);

    ASSERT_EQ(&entry, m_tree.index(mem));
}

TEST_F(index_tree_test, test_index_2)
{
    byte* mem = (byte*) m_ptrs[0];
    int expected = 0;
    m_tree.add_to_index(mem, PAGE_SIZE, &expected);

    byte* mem_end = mem + PAGE_SIZE;
    for (byte* it = mem; it < mem_end; ++it) {
        ASSERT_EQ(&expected, m_tree.index(it));
    }
}

TEST_F(index_tree_test, test_remove_index)
{
    byte* mem1 = (byte*) m_ptrs[0];
    byte* mem2 = (byte*) m_ptrs[1];

    int val1 = 0;
    int val2 = 0;
    m_tree.add_to_index(mem1, PAGE_SIZE, &val1);
    m_tree.add_to_index(mem2, PAGE_SIZE, &val2);

    m_tree.remove_from_index(mem1, PAGE_SIZE);

    ASSERT_EQ(nullptr, m_tree.index(mem1));
    ASSERT_EQ(&val2, m_tree.index(mem2));
}