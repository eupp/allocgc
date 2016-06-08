#include <gtest/gtest.h>

#include <libprecisegc/details/utils/dynarray.hpp>

using namespace precisegc::details::utils;

TEST(dynarray_test, test_constructor)
{
    dynarray<int> da1(0);
    ASSERT_TRUE(da1.empty());

    size_t size = 4;
    dynarray<int> da2(size);
    ASSERT_FALSE(da2.empty());
    ASSERT_EQ(size, da2.size());

    dynarray<int> da3(size, 1);
    ASSERT_EQ(1, da3.front());
    ASSERT_EQ(1, da3.back());
    for (size_t i = 0; i < da3.size(); ++i) {
        ASSERT_EQ(1, da3[i]);
    }

    dynarray<int> da4({0, 1, 2, 3});
    for (size_t i = 0; i < da4.size(); ++i) {
        ASSERT_EQ(i, da4[i]);
    }
}

TEST(dynarray_test, test_data)
{
    dynarray<int> da({0, 1, 2, 3});
    int* it = da.data();
    for (size_t i = 0; i < da.size(); ++i, ++it) {
        ASSERT_EQ(i, *it);
    }
}

TEST(dynarray_test, test_iterators)
{
    dynarray<int> da1(0);
    ASSERT_EQ(0, std::distance(da1.begin(), da1.end()));

    dynarray<int> da2({0, 1, 2, 3});
    ASSERT_EQ(4, std::distance(da2.begin(), da2.end()));
    int i = 0;
    for (auto it = da2.begin(); it < da2.end(); ++it, ++i) {
        ASSERT_EQ(i, *it);
    }


}