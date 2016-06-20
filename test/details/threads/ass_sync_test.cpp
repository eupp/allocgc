#include <gtest/gtest.h>

#include <thread>
#include <atomic>

#include <libprecisegc/details/threads/ass_sync.hpp>

using namespace precisegc::details::threads;

class ass_sync_test : public ::testing::Test
{
public:
    ass_sync_test()
        : thread_counter(0)
    {}

    std::atomic<int> thread_counter;
};

TEST_F(ass_sync_test, test_event)
{
    ass_event event;
    auto routine = [this, &event] {
        event.wait();
        thread_counter++;
    };

    static const size_t THREADS_CNT = 100;
    std::thread threads[THREADS_CNT];
    for (auto& thread: threads) {
        thread = std::thread(routine);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(0, thread_counter);

    event.notify(THREADS_CNT);

    for (auto& thread: threads) {
        thread.join();
    }

    EXPECT_EQ((size_t) THREADS_CNT, thread_counter);
}

TEST_F(ass_sync_test, test_barrier)
{
    ass_barrier barrier;
    auto routine = [this, &barrier] {
        thread_counter++;
        barrier.notify();
    };

    static const size_t THREADS_CNT = 100;
    std::thread threads[THREADS_CNT];
    for (auto& thread: threads) {
        thread = std::thread(routine);
    }
    barrier.wait(THREADS_CNT);

    EXPECT_EQ(thread_counter, (size_t) THREADS_CNT);

    for (auto& thread: threads) {
        thread.join();
    }
}

