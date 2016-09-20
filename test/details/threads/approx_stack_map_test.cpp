#include <gtest/gtest.h>

#include <libprecisegc/details/threads/approx_stack_map.hpp>

#include <set>

using namespace precisegc::details;
using namespace precisegc::details::threads;

TEST(approx_stack_map_test, test_register_root_1)
{
    gc_handle h1;
    gc_handle h2;

    approx_stack_map stack_map;
    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    ASSERT_EQ(2, stack_map.count());
    ASSERT_TRUE(stack_map.contains(&h1));
    ASSERT_TRUE(stack_map.contains(&h2));
}

void test_register_root_2_helper(approx_stack_map& stack_map, gc_handle* ph1, gc_handle* ph2)
{
    gc_handle h3;

    stack_map.register_root(&h3);

    ASSERT_EQ(3, stack_map.count());
    ASSERT_TRUE(stack_map.contains(ph1));
    ASSERT_TRUE(stack_map.contains(ph2));
    ASSERT_TRUE(stack_map.contains(&h3));
}

TEST(approx_stack_map_test, test_register_root_2)
{
    gc_handle h1;
    gc_handle h2;

    approx_stack_map stack_map;
    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    test_register_root_2_helper(stack_map, &h1, &h2);
}

TEST(approx_stack_map_test, test_deregister_root_1)
{
    gc_handle h1;
    gc_handle h2;

    approx_stack_map stack_map;
    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    stack_map.deregister_root(&h2);
    ASSERT_EQ(1, stack_map.count());
    ASSERT_TRUE(stack_map.contains(&h1));
    ASSERT_FALSE(stack_map.contains(&h2));

    stack_map.deregister_root(&h1);
    ASSERT_EQ(0, stack_map.count());
    ASSERT_FALSE(stack_map.contains(&h1));
    ASSERT_FALSE(stack_map.contains(&h2));
}

void test_deregister_root_2_helper(approx_stack_map& stack_map, gc_handle* ph1, gc_handle* ph2)
{
    gc_handle h3;

    stack_map.register_root(&h3);

    stack_map.deregister_root(ph1);
    stack_map.deregister_root(ph2);

    ASSERT_EQ(1, stack_map.count());
    ASSERT_FALSE(stack_map.contains(ph1));
    ASSERT_FALSE(stack_map.contains(ph2));
    ASSERT_TRUE(stack_map.contains(&h3));
}

TEST(approx_stack_map_test, test_deregister_root_2)
{
    gc_handle h1;
    gc_handle h2;

    approx_stack_map stack_map;
    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    test_deregister_root_2_helper(stack_map, &h1, &h2);
}

TEST(approx_stack_map_test, test_trace)
{
    gc_handle h1;
    gc_handle h2;
    gc_handle h3;

    std::set<gc_handle*> expected({&h1, &h2, &h3});
    std::set<gc_handle*> actual;

    approx_stack_map stack_map;
    stack_map.register_root(&h1);
    stack_map.register_root(&h2);
    stack_map.register_root(&h3);

    stack_map.trace([&actual] (gc_handle* root) { actual.insert(root); });

    ASSERT_EQ(expected, actual);
}
