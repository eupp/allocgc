#include <gtest/gtest.h>

#include "liballocgc/details/allocators/pow2_bucket_policy.hpp"

using namespace allocgc::details::allocators;

TEST(pow2_bucket_policy_test, test_1)
{
    pow2_bucket_policy<0, 3> bucket_policy;
    ASSERT_EQ(4, (size_t) bucket_policy.BUCKET_COUNT);

    ASSERT_EQ(0, bucket_policy.bucket(1));
    ASSERT_EQ(1, bucket_policy.bucket(2));
    ASSERT_EQ(2, bucket_policy.bucket(3));
    ASSERT_EQ(2, bucket_policy.bucket(4));
    ASSERT_EQ(3, bucket_policy.bucket(5));
    ASSERT_EQ(3, bucket_policy.bucket(6));
    ASSERT_EQ(3, bucket_policy.bucket(7));
    ASSERT_EQ(3, bucket_policy.bucket(8));

    ASSERT_EQ(1, bucket_policy.bucket_size(0));
    ASSERT_EQ(2, bucket_policy.bucket_size(1));
    ASSERT_EQ(4, bucket_policy.bucket_size(2));
    ASSERT_EQ(8, bucket_policy.bucket_size(3));
}

TEST(pow2_bucket_policy_test, test_2)
{
    pow2_bucket_policy<5, 8> bucket_policy;
    ASSERT_EQ(4, (size_t) bucket_policy.BUCKET_COUNT);

    ASSERT_EQ(0, bucket_policy.bucket(32));
    ASSERT_EQ(1, bucket_policy.bucket(48));
    ASSERT_EQ(1, bucket_policy.bucket(64));
    ASSERT_EQ(2, bucket_policy.bucket(96));
    ASSERT_EQ(2, bucket_policy.bucket(128));
    ASSERT_EQ(3, bucket_policy.bucket(196));
    ASSERT_EQ(3, bucket_policy.bucket(256));

    ASSERT_EQ(32, bucket_policy.bucket_size(0));
    ASSERT_EQ(64, bucket_policy.bucket_size(1));
    ASSERT_EQ(128, bucket_policy.bucket_size(2));
    ASSERT_EQ(256, bucket_policy.bucket_size(3));
}