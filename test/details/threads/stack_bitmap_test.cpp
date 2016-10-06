#include <gtest/gtest.h>

#include <libprecisegc/details/threads/stack_bitmap.hpp>
#include <libprecisegc/details/threads/return_address.hpp>

#include <set>
#include <cstdlib>

using namespace precisegc::details;
using namespace precisegc::details::threads;

void test_register_root_1_helper(stack_bitmap& stack_map)
{
    gc_word h1;
    gc_word h2;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    ASSERT_EQ(2, stack_map.count());
    ASSERT_TRUE(stack_map.contains(&h1));
    ASSERT_TRUE(stack_map.contains(&h2));
}

TEST(stack_bitmap_test, test_register_root_1)
{
    gc_word h1;
    gc_word h2;

    stack_bitmap stack_map(frame_address());
    test_register_root_1_helper(stack_map);
}

void test_register_root_2_helper_2(stack_bitmap& stack_map, gc_word* ph1, gc_word* ph2)
{
    gc_word h3;

    stack_map.register_root(&h3);

    ASSERT_EQ(3, stack_map.count());
    ASSERT_TRUE(stack_map.contains(ph1));
    ASSERT_TRUE(stack_map.contains(ph2));
    ASSERT_TRUE(stack_map.contains(&h3));
}

void test_register_root_2_helper_1(stack_bitmap& stack_map)
{
    gc_word h1;
    gc_word h2;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    test_register_root_2_helper_2(stack_map, &h1, &h2);
}

TEST(stack_bitmap_test, test_register_root_2)
{
    stack_bitmap stack_map(frame_address());
    test_register_root_2_helper_1(stack_map);
}

void test_register_root_3_helper(stack_bitmap& stack_map)
{
    static const size_t SIZE = 2 * stack_bitmap::STACK_FRAME_SIZE;
    gc_word handles[SIZE];

    for (size_t i = 0; i < SIZE; ++i) {
        stack_map.register_root(&handles[i]);
    }

    EXPECT_EQ(SIZE, stack_map.count());
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_TRUE(stack_map.contains(&handles[i]));
    }
}

TEST(stack_bitmap_test, test_register_root_3)
{
    stack_bitmap stack_map(frame_address());
    test_register_root_3_helper(stack_map);
}

void test_deregister_root_1_helper(stack_bitmap& stack_map)
{
    gc_word h1;
    gc_word h2;

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

TEST(stack_bitmap_test, test_deregister_root_1)
{
    stack_bitmap stack_map(frame_address());
    test_deregister_root_1_helper(stack_map);
}

void test_deregister_root_2_helper_2(stack_bitmap& stack_map, gc_word* ph1, gc_word* ph2)
{
    gc_word h3;

    stack_map.register_root(&h3);

    stack_map.deregister_root(ph2);
    stack_map.deregister_root(ph1);

    ASSERT_EQ(1, stack_map.count());
    ASSERT_FALSE(stack_map.contains(ph1));
    ASSERT_FALSE(stack_map.contains(ph2));
    ASSERT_TRUE(stack_map.contains(&h3));
}

void test_deregister_root_2_helper_1(stack_bitmap& stack_map)
{
    gc_word h1;
    gc_word h2;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);

    test_register_root_2_helper_2(stack_map, &h1, &h2);
}

TEST(stack_bitmap_test, test_deregister_root_2)
{
    stack_bitmap stack_map(frame_address());
    test_deregister_root_2_helper_1(stack_map);
}

void test_deregister_root_3_helper(stack_bitmap& stack_map)
{
    static const size_t SIZE = 2 * stack_bitmap::STACK_FRAME_SIZE;
    gc_word handles[SIZE];

    for (size_t i = 0; i < SIZE; ++i) {
        stack_map.register_root(&handles[i]);
    }

    for (size_t i = 0; i < SIZE; ++i) {
        stack_map.deregister_root(&handles[i]);
    }

    EXPECT_EQ(0, stack_map.count());
    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_FALSE(stack_map.contains(&handles[i]));
    }
}

TEST(stack_bitmap_test, test_deregister_root_3)
{
    stack_bitmap stack_map(frame_address());
    test_register_root_3_helper(stack_map);
}

void test_trace_helper(stack_bitmap& stack_map)
{
    gc_word h1;
    gc_word h2;
    gc_word h3;

    std::set<gc_word*> expected({&h1, &h2, &h3});
    std::set<gc_word*> actual;

    stack_map.register_root(&h1);
    stack_map.register_root(&h2);
    stack_map.register_root(&h3);

    stack_map.trace([&actual] (gc_word* root) { actual.insert(root); });

    ASSERT_EQ(expected, actual);
}

TEST(stack_bitmap_test, test_trace)
{
    stack_bitmap stack_map(frame_address());
    test_trace_helper(stack_map);
}
