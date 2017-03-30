#include <gtest/gtest.h>

#include <functional>
#include <array>

#include <liballocgc/details/utils/static_thread_pool.hpp>

using namespace allocgc::details::utils;

TEST(static_thread_pool_test, test_run)
{
    static const int THREADS_CNT = 4;
    int x[THREADS_CNT] = {0};

    static_thread_pool thread_pool(THREADS_CNT);
    std::array<std::function<void()>, THREADS_CNT> tasks;
    for (int i = 0; i < THREADS_CNT; ++i) {
        tasks[i] = [i, &x] { x[i] = i; };
    }
    thread_pool.run(tasks.begin(), tasks.end());

    for (int i = 0; i < THREADS_CNT; ++i) {
        ASSERT_EQ(i, x[i]);
    }
}
