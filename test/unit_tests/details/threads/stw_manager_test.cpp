#include <gtest/gtest.h>

#include <libprecisegc/details/threads/stw_manager.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

using namespace precisegc::details::utils;
using namespace precisegc::details::threads;

TEST(stw_manager_test, test_stw)
{
    std::atomic<bool> thread_exit(false);
    auto routine = [&thread_exit] {
        while (!thread_exit) {};
    };

    static const size_t THREADS_CNT = 10;
    scoped_thread threads[THREADS_CNT];
    for (auto& thread: threads) {
        thread = std::thread(routine);
    }

    stw_manager& stwm = stw_manager::instance();

    for (auto& thread: threads) {
        stwm.suspend_thread(thread.native_handle());
    }
    stwm.wait_for_world_stop();

    EXPECT_EQ(THREADS_CNT, stwm.threads_suspended());

    for (auto& thread: threads) {
        stwm.resume_thread(thread.native_handle());
    }
    stwm.wait_for_world_start();

    EXPECT_EQ(0, stwm.threads_suspended());

    thread_exit = true;
}
