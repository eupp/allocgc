#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/gc_facade.hpp>

#include "incremental_gc_mock.hpp"
#include "initiation_policy_mock.hpp"

using namespace precisegc::details;

using ::testing::_;
using ::testing::Exactly;
using ::testing::Return;
using ::testing::AllOf;
using ::testing::Field;

//struct garbage_collector_test : public ::testing::Test
//{
//    garbage_collector_test()
//    {
//        auto gc_strategy_owner = utils::make_unique<incremental_gc_mock>();
//        auto policy_mock_owner = utils::make_unique<initiation_policy_mock>();
//
//        gc_mock = gc_strategy_owner.get();
//        policy_mock = policy_mock_owner.get();
//
//        collector.init(std::move(gc_strategy_owner), std::move(policy_mock_owner));
//    }
//
//    gc_facade collector;
//    incremental_gc_mock* gc_mock;
//    initiation_policy_mock* policy_mock;
//};
//
//TEST_F(garbage_collector_test, test_initiation_point_user_request)
//{
//    EXPECT_CALL(*gc_mock, gc(_))
//            .Times(Exactly(1));
//
//    collector.initiation_point(initiation_point_type::USER_REQUEST);
//}

//TEST_F(garbage_collector_test, test_initiation_point_idle)
//{
//    EXPECT_CALL(*policy_mock, check(initiation_point_type::HEAP_EXPANSION, _))
//            .WillRepeatedly(Return(gc_phase::IDLE));
//
//    EXPECT_CALL(*gc_mock, gc(_))
//            .Times(0);
//
//    collector.initiation_point(initiation_point_type::HEAP_EXPANSION);
//}
//
//TEST_F(garbage_collector_test, test_initiation_point_mark)
//{
//    EXPECT_CALL(*policy_mock, check(initiation_point_type::HEAP_EXPANSION, _))
//            .WillRepeatedly(Return(gc_phase::MARK));
//
//    EXPECT_CALL(*gc_mock, gc(gc_phase::MARK))
//            .Times(Exactly(1));
//
//    collector.initiation_point(initiation_point_type::HEAP_EXPANSION);
//}
//
//TEST_F(garbage_collector_test, test_initiation_point_sweep)
//{
//    EXPECT_CALL(*policy_mock, check(initiation_point_type::HEAP_EXPANSION, _))
//            .WillRepeatedly(Return(gc_phase::COLLECT));
//
//    EXPECT_CALL(*gc_mock, gc(gc_phase::COLLECT))
//            .Times(Exactly(1));
//
//    collector.initiation_point(initiation_point_type::HEAP_EXPANSION);
//}
//
//TEST_F(garbage_collector_test, test_register_sweep)
//{
//    EXPECT_CALL(*policy_mock, update(_))
//            .Times(Exactly(1));
//
//    collector.register_sweep(gc_sweep_stat(), gc_pause_stat());
//}