#include <gtest/gtest.h>

#include <libprecisegc/details/collectors/pin_set.hpp>

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::collectors;

TEST(pin_set_test, test_insert)
{
    pin_set rs;

    byte* p1;
    byte* p2;

    rs.insert(p1);
    ASSERT_TRUE(rs.contains(p1));

    rs.insert(p2);
    ASSERT_TRUE(rs.contains(p1));
    ASSERT_TRUE(rs.contains(p2));
}

TEST(pin_set_test, test_remove)
{
    pin_set rs;

    byte a, b, c;

    byte* p1 = &a;
    byte* p2 = &b;
    byte* p3 = &c;

    rs.insert(p1);
    rs.insert(p2);
    rs.insert(p3);

    rs.remove(p3);
    ASSERT_FALSE(rs.contains(p3));

    rs.remove(p1);
    ASSERT_FALSE(rs.contains(p1));

    ASSERT_TRUE(rs.contains(p2));
}
