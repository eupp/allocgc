#include <gtest/gtest.h>

#include <type_traits>
#include <liballocgc/gc_common.hpp>


using namespace allocgc;

TEST(gc_handle_test, test_podness)
{
    ASSERT_TRUE(std::is_pod<gc_handle>::value);
}

