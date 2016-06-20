#include <gtest/gtest.h>

#include <libprecisegc/details/stack_map.hpp>

using namespace precisegc::details;
using namespace precisegc::details::ptrs;

TEST(stack_map_test, test_insert)
{
    stack_map<int> rs;

    rs.insert(1);
    ASSERT_TRUE(rs.contains(1));

    rs.insert(2);
    ASSERT_TRUE(rs.contains(1));
    ASSERT_TRUE(rs.contains(2));
}

TEST(stack_map_test, test_remove)
{
    stack_map<int> rs;

    rs.insert(1);
    rs.insert(2);
    rs.insert(3);

    rs.remove(3);
    ASSERT_FALSE(rs.contains(3));

    rs.remove(1);
    ASSERT_FALSE(rs.contains(1));

    ASSERT_TRUE(rs.contains(2));
}
