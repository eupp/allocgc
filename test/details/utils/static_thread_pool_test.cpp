#include <gtest/gtest.h>

#include <functional>
#include <array>

#include <libprecisegc/details/utils/static_thread_pool.hpp>

using namespace precisegc::details::utils;

TEST(static_thread_pool_test, test_submit)
{
    static const int THREADS_CNT = 4;
    int x[THREADS_CNT] = {0};

    static_thread_pool thread_pool(THREADS_CNT);
    for (int i = 0; i < THREADS_CNT; ++i) {
        thread_pool.submit([i, &x] { x[i] = i; });
    }
    thread_pool.wait_for_complete();

    for (int i = 0; i < THREADS_CNT; ++i) {
        ASSERT_EQ(i, x[i]);
    }
}

TEST(static_thread_pool_test, test_submit_range)
{
    static const int THREADS_CNT = 4;
    int x[THREADS_CNT] = {0};

    static_thread_pool thread_pool(THREADS_CNT);
    std::array<std::function<void()>, THREADS_CNT> tasks;
    for (int i = 0; i < THREADS_CNT; ++i) {
        tasks[i] = [i, &x] { x[i] = i; };
    }
    thread_pool.submit(tasks.begin(), tasks.end());
    thread_pool.wait_for_complete();

    for (int i = 0; i < THREADS_CNT; ++i) {
        ASSERT_EQ(i, x[i]);
    }
}
