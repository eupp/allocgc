#include <gtest/gtest.h>

#include <libprecisegc/details/collectors/initation_policy.hpp>

using namespace precisegc::details;
using namespace precisegc::details::collectors;

struct space_base_policy_test : public ::testing::Test
{
    space_base_policy_test()
        : policy(32, 0.5, 2.0, 96)
    {}

    space_based_policy policy;
    gc_state stat;
};

TEST_F(space_base_policy_test, test_check)
{
    stat.heap_size = 0.5 * policy.threshold() * policy.heap_size();
    ASSERT_FALSE(policy.check(stat, initation_point_type::AFTER_ALLOC));

    stat.heap_size = policy.threshold() * policy.heap_size() + 1;
    ASSERT_TRUE(policy.check(stat, initation_point_type::AFTER_ALLOC));
}

TEST_F(space_base_policy_test, test_update)
{
    stat.heap_size = policy.heap_size();
    policy.update(stat, initation_point_type::AFTER_ALLOC);
    ASSERT_EQ(stat.heap_size * policy.increase_factor(), policy.heap_size());

    stat.heap_size = policy.heap_size();
    policy.update(stat, initation_point_type::AFTER_ALLOC);
    ASSERT_EQ(policy.max_heap_size(), policy.heap_size());

    stat.heap_size = 2 * policy.max_heap_size();
    ASSERT_THROW(policy.update(stat, initation_point_type::AFTER_ALLOC), gc_bad_alloc);
}

struct incremental_space_base_policy_test : public ::testing::Test
{
    incremental_space_base_policy_test()
        : policy(32, 0.25, 0.5, 2.0, 96)
    {}

    incremental_space_based_policy policy;
    gc_state stat;
};

TEST_F(incremental_space_base_policy_test, test_check)
{
    stat.heap_size = 0.5 * policy.heap_size() * policy.marking_threshold();
    ASSERT_EQ(gc_phase::IDLING, policy.check(stat, initation_point_type::AFTER_ALLOC));

    stat.heap_size = policy.heap_size() * policy.marking_threshold() + 1;
    ASSERT_EQ(gc_phase::MARKING, policy.check(stat, initation_point_type::AFTER_ALLOC));

    stat.heap_size = policy.heap_size() * policy.sweeping_threshold() + 1;
    ASSERT_EQ(gc_phase::SWEEPING, policy.check(stat, initation_point_type::AFTER_ALLOC));
}