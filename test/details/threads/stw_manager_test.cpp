#include <gtest/gtest.h>

#include <liballocgc/details/threads/stw_manager.hpp>
#include <liballocgc/details/utils/scoped_thread.hpp>
#include <liballocgc/details/utils/scope_guard.hpp>

#include <thread>
#include <atomic>

using namespace allocgc::details::utils;
using namespace allocgc::details::threads;

TEST(stw_manager_test, test_stw)
{
    std::atomic<size_t> thread_started_cnt(0);
    std::atomic<bool> thread_exit(false);
    auto guard = make_scope_guard([&thread_exit] { thread_exit = true; });
    auto routine = [&thread_started_cnt, &thread_exit] {
        ++thread_started_cnt;
        while (!thread_exit) {};
    };

    static const size_t THREADS_CNT = 10;
    scoped_thread threads[THREADS_CNT];
    for (auto& thread: threads) {
        thread = std::thread(routine);
    }

    while (thread_started_cnt < THREADS_CNT) {
        std::this_thread::yield();
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
    guard.commit();
}
