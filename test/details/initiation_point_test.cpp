#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/garbage_collector.hpp>

#include "incremental_gc_mock.hpp"
#include "initiation_policy_mock.hpp"

using namespace precisegc::details;

using ::testing::_;
using ::testing::Exactly;
using ::testing::Return;
using ::testing::AllOf;
using ::testing::Field;

struct initiation_point_test : public ::testing::Test
{
    initiation_point_test()
    {
        auto gc_strategy_owner = utils::make_unique<incremental_gc_mock>();
        auto policy_mock_owner = utils::make_unique<initiation_policy_mock>();

        gc_mock = gc_strategy_owner.get();
        policy_mock = policy_mock_owner.get();

        collector.init(std::move(gc_strategy_owner), std::move(policy_mock_owner));
    }

    garbage_collector collector;
    incremental_gc_mock* gc_mock;
    initiation_policy_mock* policy_mock;
};

TEST_F(initiation_point_test, test_user_request)
{
    EXPECT_CALL(*gc_mock, gc(gc_phase::SWEEP))
            .Times(Exactly(1));

    EXPECT_CALL(*policy_mock, update(_))
            .Times(Exactly(1));

    collector.initiation_point(initiation_point_type::USER_REQUEST);
}

// check IDLE -> IDLE transition
TEST_F(initiation_point_test, test_idle)
{
    EXPECT_CALL(*policy_mock, check(initiation_point_type::HEAP_GROWTH, _))
            .WillRepeatedly(Return(gc_phase::IDLE));

    EXPECT_CALL(*gc_mock, gc(_))
            .Times(0);

    collector.initiation_point(initiation_point_type::AFTER_ALLOC);
}

// check IDLE -> MARK transition
TEST_F(initiation_point_test, test_mark)
{
    EXPECT_CALL(*policy_mock, check(initiation_point_type::HEAP_GROWTH, _))
            .WillRepeatedly(Return(gc_phase::MARK));

    EXPECT_CALL(*gc_mock, gc(gc_phase::MARK))
            .Times(Exactly(1));

    collector.initiation_point(initiation_point_type::AFTER_ALLOC);
}

// check IDLE -> SWEEP transition
TEST_F(initiation_point_test, test_sweep)
{
    EXPECT_CALL(*policy_mock, check(initiation_point_type::HEAP_GROWTH, _))
            .WillRepeatedly(Return(gc_phase::SWEEP));

    EXPECT_CALL(*gc_mock, gc(gc_phase::SWEEP))
            .Times(Exactly(1));

    collector.initiation_point(initiation_point_type::HEAP_GROWTH);
}