#include <gtest/gtest.h>

#include <libprecisegc/details/initiation_policy.hpp>

#include "serial_gc_mock.hpp"

using namespace precisegc::details;

//struct space_base_policy_test : public ::testing::Test
//{
//    space_base_policy_test()
//        : policy(&gc_mock, 32, 0.25, 0.5, 2.0, 96)
//    {}
//
//    serial_gc_mock gc_mock;
//    space_based_policy policy;
//};
//
//TEST_F(space_base_policy_test, test_initiation_point)
//{
//    state.heap_size = 0.5 * policy.marking_threshold() * policy.heap_size();
//    ASSERT_EQ(gc_phase::IDLE, policy.check(initiation_point_type::HEAP_EXPANSION,
//                                           initiation_point_data::create_empty_data(),
//                                           state));
//
//    state.heap_size = policy.heap_size() * policy.marking_threshold() + 1;
//    ASSERT_EQ(gc_phase::MARK, policy.check(initiation_point_type::HEAP_EXPANSION,
//                                           initiation_point_data::create_empty_data(),
//                                           state));
//
//    state.heap_size = policy.heap_size() * policy.sweeping_threshold() + 1;
//    ASSERT_EQ(gc_phase::SWEEP, policy.check(initiation_point_type::HEAP_EXPANSION,
//                                            initiation_point_data::create_empty_data(),
//                                            state));
//}
//
//TEST_F(space_base_policy_test, test_update)
//{
//    state.heap_size = policy.heap_size();
//    policy.update(state);
//    ASSERT_EQ(state.heap_size * policy.increase_factor(), policy.heap_size());
//
//    state.heap_size = policy.heap_size();
//    policy.update(state);
//    ASSERT_EQ(policy.max_heap_size(), policy.heap_size());
//
//    state.heap_size = 2 * policy.max_heap_size();
//    ASSERT_THROW(policy.update(state), gc_bad_alloc);
//}