#include <gtest/gtest.h>

#include <liballocgc/details/utils/dynarray.hpp>

using namespace allocgc::details::utils;

TEST(dynarray_test, test_constructor)
{
    dynarray<int> da1(0);
    ASSERT_TRUE(da1.empty());

    dynarray<int> da2(4);
    ASSERT_FALSE(da2.empty());
    ASSERT_EQ(4, da2.size());

    dynarray<int> da3((size_t) 4, 1);
    ASSERT_EQ(1, da3.front());
    ASSERT_EQ(1, da3.back());
    for (size_t i = 0; i < 4; ++i) {
        ASSERT_EQ(1, da3[i]);
    }

    dynarray<int> da4({0, 1, 2, 3});
    ASSERT_EQ(da4.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        ASSERT_EQ(i, da4[i]);
    }
}

TEST(dynarray_test, test_data)
{
    dynarray<int> da({0, 1, 2, 3});
    int* it = da.data();
    for (size_t i = 0; i < 4; ++i, ++it) {
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

TEST(dynarray_test, test_const_iterators)
{
    dynarray<int> da1(0);
    ASSERT_EQ(0, std::distance(da1.cbegin(), da1.cend()));

    dynarray<int> da2({0, 1, 2, 3});
    ASSERT_EQ(4, std::distance(da2.cbegin(), da2.cend()));
    int i = 0;
    for (auto it = da2.cbegin(); it < da2.cend(); ++it, ++i) {
        ASSERT_EQ(i, *it);
    }
}