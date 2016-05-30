#include <gtest/gtest.h>

#include <libprecisegc/details/root_set.hpp>

using namespace precisegc::details;
using namespace precisegc::details::ptrs;

TEST(root_set_test, test_add)
{
    root_set rs;

    gc_untyped_ptr p1;
    rs.add(&p1);
    ASSERT_TRUE(rs.is_root(&p1));

    gc_untyped_ptr p2;
    rs.add(&p2);
    ASSERT_TRUE(rs.is_root(&p1));
    ASSERT_TRUE(rs.is_root(&p2));
}

TEST(root_set_test, test_remove)
{
    root_set rs;

    gc_untyped_ptr p1, p2, p3;
    rs.add(&p1);
    rs.add(&p2);
    rs.add(&p3);

    rs.remove(&p3);
    ASSERT_FALSE(rs.is_root(&p3));

    rs.remove(&p1);
    ASSERT_FALSE(rs.is_root(&p1));

    ASSERT_TRUE(rs.is_root(&p2));
}
