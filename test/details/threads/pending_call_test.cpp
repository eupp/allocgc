#include <gtest/gtest.h>

#include <liballocgc/details/threads/pending_call.hpp>
#include <liballocgc/details/utils/scope_guard.hpp>

using namespace allocgc::details::threads;
using namespace allocgc::details::utils;

struct pending_call_test : public ::testing::Test
{
    pending_call_test()
    {
        counter = 0;
    }

    static void inc_counter()
    {
        ++counter;
    }

    static int counter;
};

int pending_call_test::counter = 0;

TEST_F(pending_call_test, test_pending_call)
{
    pending_call pcall(pending_call_test::inc_counter);
    ASSERT_EQ(0, (int) counter);

    pcall();
    ASSERT_EQ(1, (int) counter);

    {
        pcall.enter_pending_scope();
        auto guard = make_scope_guard([&pcall] { pcall.leave_pending_scope(); });
        pcall();
        ASSERT_EQ(1, (int) counter);
        ASSERT_TRUE(pcall.is_in_pending_scope());
    }
    ASSERT_EQ(2, (int) counter);
}

TEST_F(pending_call_test, test_safe_scope)
{
    pending_call pcall(pending_call_test::inc_counter);
    ASSERT_EQ(0, (int) counter);

    pcall.enter_pending_scope();
    auto guard1 = make_scope_guard([&pcall] { pcall.leave_pending_scope(); });
    pcall();
    ASSERT_EQ(0, (int) counter);

    pcall.enter_safe_scope();
    auto guard2 = make_scope_guard([&pcall] { pcall.leave_safe_scope(); });
    ASSERT_FALSE(pcall.is_in_pending_scope());
    ASSERT_EQ(1, (int) counter);
}
