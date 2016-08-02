#include <gtest/gtest.h>

#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/utils/barrier.hpp>

#include <thread>
#include <atomic>

using namespace precisegc::details::utils;

TEST(barrier_test, test_barrier)
{
    const size_t THREADS_CNT = 10;

    barrier b(THREADS_CNT + 1);
    std::atomic<size_t> threads_resumed{0};

    scoped_thread threads[THREADS_CNT];
    for (auto& thread: threads) {
        thread = std::thread([&b, &threads_resumed] {
            b.wait();
            ++threads_resumed;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(0, threads_resumed);
    b.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(THREADS_CNT, threads_resumed);
}