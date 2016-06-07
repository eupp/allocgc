#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/initator.hpp>

#include "serial_gc_mock.hpp"
#include "incremental_gc_mock.hpp"
#include "initation_policy_mock.hpp"
#include "incremental_initation_policy_mock.hpp"

using namespace precisegc::details;

using ::testing::_;
using ::testing::Exactly;
using ::testing::Return;
using ::testing::AllOf;
using ::testing::Field;

struct initator_test : public ::testing::Test
{
    initator_test()
        : initor(&gc_mock, utils::make_unique<initation_policy_mock>())
        , policy_mock(static_cast<initation_policy_mock*>(initor.get_policy()))
    {}

    serial_gc_mock gc_mock;
    initator initor;
    initation_policy_mock* policy_mock;
};

TEST_F(initator_test, test_user_request)
{
    EXPECT_CALL(gc_mock, gc())
            .Times(Exactly(1));

    EXPECT_CALL(*policy_mock, update(_, initation_point_type::USER_REQUEST))
            .Times(Exactly(1));

    initor.initation_point(initation_point_type::USER_REQUEST);
}

// check gc is not called when policy returns false
TEST_F(initator_test, test_initation_point_1)
{
    EXPECT_CALL(gc_mock, gc())
            .Times(0);

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(false));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}

// check gc is called when policy returns true
TEST_F(initator_test, test_initation_point_2)
{
    EXPECT_CALL(gc_mock, gc())
            .Times(Exactly(1));

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(true));

    EXPECT_CALL(*policy_mock, update(_, initation_point_type::AFTER_ALLOC))
            .Times(Exactly(1));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}

struct incremental_initator_test : public ::testing::Test
{
    incremental_initator_test()
        : initor(&gc_mock, utils::make_unique<incremental_initation_policy_mock>())
        , policy_mock(static_cast<incremental_initation_policy_mock*>(initor.get_policy()))
    {}

    incremental_gc_mock gc_mock;
    incremental_initator initor;
    incremental_initation_policy_mock* policy_mock;
};


TEST_F(incremental_initator_test, test_user_request)
{
    EXPECT_CALL(gc_mock, gc())
            .Times(Exactly(1));

    EXPECT_CALL(gc_mock, phase())
            .WillRepeatedly(Return(gc_phase::IDLING));

    EXPECT_CALL(*policy_mock, update(_, initation_point_type::USER_REQUEST))
            .Times(Exactly(1));

    initor.initation_point(initation_point_type::USER_REQUEST);
}

// check IDLING -> IDLING transition
TEST_F(incremental_initator_test, test_initation_point_1)
{
    EXPECT_CALL(gc_mock, gc_increment(_))
            .Times(0);

    EXPECT_CALL(gc_mock, phase())
            .WillRepeatedly(Return(gc_phase::IDLING));

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(gc_phase::IDLING));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}

// check IDLING -> MARKING transition
TEST_F(incremental_initator_test, test_initation_point_2)
{
    EXPECT_CALL(gc_mock, gc_increment(Field(&incremental_gc_ops::phase, gc_phase::MARKING)))
            .Times(Exactly(1));

    EXPECT_CALL(gc_mock, phase())
            .WillRepeatedly(Return(gc_phase::IDLING));

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(gc_phase::MARKING));

    EXPECT_CALL(*policy_mock, update(_, initation_point_type::AFTER_ALLOC))
            .Times(Exactly(1));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}

// check IDLING -> SWEEPING transition
TEST_F(incremental_initator_test, test_initation_point_3)
{
    EXPECT_CALL(gc_mock, gc_increment(Field(&incremental_gc_ops::phase, gc_phase::SWEEPING)))
            .Times(Exactly(1));

    EXPECT_CALL(gc_mock, phase())
            .WillRepeatedly(Return(gc_phase::IDLING));

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(gc_phase::SWEEPING));

    EXPECT_CALL(*policy_mock, update(_, initation_point_type::AFTER_ALLOC))
            .Times(Exactly(1));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}

// check MARKING -> SWEEPING transition
TEST_F(incremental_initator_test, test_initation_point_4)
{
    EXPECT_CALL(gc_mock, gc_increment(Field(&incremental_gc_ops::phase, gc_phase::SWEEPING)))
            .Times(Exactly(1));

    EXPECT_CALL(gc_mock, phase())
            .WillRepeatedly(Return(gc_phase::MARKING));

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(gc_phase::SWEEPING));

    EXPECT_CALL(*policy_mock, update(_, initation_point_type::AFTER_ALLOC))
            .Times(Exactly(1));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}

// check MARKING -> MARKING transition
TEST_F(incremental_initator_test, test_initation_point_5)
{
    EXPECT_CALL(gc_mock, gc_increment(_))
            .Times(0);

    EXPECT_CALL(gc_mock, phase())
            .WillRepeatedly(Return(gc_phase::MARKING));

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(gc_phase::MARKING));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}

// check SWEEPING -> SWEEPING transition
TEST_F(incremental_initator_test, test_initation_point_6)
{
    EXPECT_CALL(gc_mock, gc_increment(_))
            .Times(0);

    EXPECT_CALL(gc_mock, phase())
            .WillRepeatedly(Return(gc_phase::SWEEPING));

    EXPECT_CALL(*policy_mock, check(_, initation_point_type::AFTER_ALLOC))
            .WillRepeatedly(Return(gc_phase::SWEEPING));

    initor.initation_point(initation_point_type::AFTER_ALLOC);
}