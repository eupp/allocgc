#include <gtest/gtest.h>

#include <type_traits>

#include <libprecisegc/gc_handle.hpp>

using namespace precisegc;
using namespace precisegc::details;

TEST(gc_handle_test, test_podness)
{
    ASSERT_TRUE(std::is_pod<gc_handle>::value);
}

