#include <gtest/gtest.h>

#include <libprecisegc/details/threads/approx_stack_map.hpp>
#include <libprecisegc/details/threads/return_address.hpp>

#include <set>
#include <cstdlib>

using namespace precisegc::details;
using namespace precisegc::details::threads;

void test_register_root_1_helper(approx_stack_map& stack_map)
{
    gc_handle h1;
    gc_handle h2;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    ASSERT_EQ(2, stack_map.count());
    ASSERT_TRUE(stack_map.contains(&h1));
    ASSERT_TRUE(stack_map.contains(&h2));
}

TEST(approx_stack_map_test, test_register_root_1)
{
    gc_handle h1;
    gc_handle h2;

    approx_stack_map stack_map(frame_address());
    test_register_root_1_helper(stack_map);
}

void test_register_root_2_helper_2(approx_stack_map& stack_map, gc_handle* ph1, gc_handle* ph2)
{
    gc_handle h3;

    stack_map.register_root(&h3);

    ASSERT_EQ(3, stack_map.count());
    ASSERT_TRUE(stack_map.contains(ph1));
    ASSERT_TRUE(stack_map.contains(ph2));
    ASSERT_TRUE(stack_map.contains(&h3));
}

void test_register_root_2_helper_1(approx_stack_map& stack_map)
{
    gc_handle h1;
    gc_handle h2;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    test_register_root_2_helper_2(stack_map, &h1, &h2);
}

TEST(approx_stack_map_test, test_register_root_2)
{
    approx_stack_map stack_map(frame_address());
    test_register_root_2_helper_1(stack_map);
}

void test_register_root_3_helper(approx_stack_map& stack_map)
{
    static const size_t SIZE = 2 * approx_stack_map::STACK_FRAME_SIZE;
    gc_handle handles[SIZE];

    srand(time(0));
    std::set<size_t> used;
    while (used.size() < SIZE) {
        size_t rnd = rand() % SIZE;
        while (used.count(rnd)) {
            rnd = rand() % SIZE;
        }
        used.insert(rnd);
        stack_map.register_root(&handles[rnd]);
    }

    EXPECT_EQ(SIZE, stack_map.count());
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_TRUE(stack_map.contains(&handles[i]));
    }
}

TEST(approx_stack_map_test, test_register_root_3)
{
    approx_stack_map stack_map(frame_address());
    test_register_root_3_helper(stack_map);
}

void test_deregister_root_1_helper(approx_stack_map& stack_map)
{
    gc_handle h1;
    gc_handle h2;

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

TEST(approx_stack_map_test, test_deregister_root_1)
{
    approx_stack_map stack_map(frame_address());
    test_deregister_root_1_helper(stack_map);
}

void test_deregister_root_2_helper_2(approx_stack_map& stack_map, gc_handle* ph1, gc_handle* ph2)
{
    gc_handle h3;

    stack_map.register_root(&h3);

    stack_map.deregister_root(ph2);
    stack_map.deregister_root(ph1);

    ASSERT_EQ(1, stack_map.count());
    ASSERT_FALSE(stack_map.contains(ph1));
    ASSERT_FALSE(stack_map.contains(ph2));
    ASSERT_TRUE(stack_map.contains(&h3));
}

void test_deregister_root_2_helper_1(approx_stack_map& stack_map)
{
    gc_handle h1;
    gc_handle h2;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    test_register_root_2_helper_2(stack_map, &h1, &h2);
}

TEST(approx_stack_map_test, test_deregister_root_2)
{
    approx_stack_map stack_map(frame_address());
    test_deregister_root_2_helper_1(stack_map);
}

void test_deregister_root_3_helper(approx_stack_map& stack_map)
{
    static const size_t SIZE = 2 * approx_stack_map::STACK_FRAME_SIZE;
    gc_handle handles[SIZE];

    for (size_t i = 0; i < SIZE; ++i) {
        stack_map.register_root(&handles[i]);
    }

    srand(time(0));
    std::set<size_t> used;
    while (used.size() < SIZE) {
        size_t rnd = rand() % SIZE;
        while (used.count(rnd)) {
            rnd = rand() % SIZE;
        }
        used.insert(rnd);
        stack_map.deregister_root(&handles[rnd]);
    }

    EXPECT_EQ(0, stack_map.count());
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_FALSE(stack_map.contains(&handles[i]));
    }
}

TEST(approx_stack_map_test, test_deregister_root_3)
{
    approx_stack_map stack_map(frame_address());
    test_register_root_3_helper(stack_map);
}

void test_trace_helper(approx_stack_map& stack_map)
{
    gc_handle h1;
    gc_handle h2;
    gc_handle h3;

    std::set<gc_handle*> expected({&h1, &h2, &h3});
    std::set<gc_handle*> actual;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);
    stack_map.register_root(&h3);

    stack_map.trace([&actual] (gc_handle* root) { actual.insert(root); });

    ASSERT_EQ(expected, actual);
}

TEST(approx_stack_map_test, test_trace)
{
    approx_stack_map stack_map(frame_address());
    test_trace_helper(stack_map);
}
