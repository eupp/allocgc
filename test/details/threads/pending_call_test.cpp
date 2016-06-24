#include <gtest/gtest.h>

#include <libprecisegc/details/threads/pending_call.hpp>

#include <mutex>

using namespace precisegc::details::threads;

namespace {
int counter = 0;

void f()
{
    ++counter;
}
}

TEST(pending_call_test, test_pending_call)
{
    pending_call pcall(f);
    ASSERT_EQ(0, counter);

    pcall();
    ASSERT_EQ(1, counter);

    {
        std::lock_guard<pending_call> lock(pcall);
        pcall();
        ASSERT_EQ(1, counter);
        ASSERT_TRUE(pcall.is_locked());
    }
    ASSERT_EQ(2, counter);
}
