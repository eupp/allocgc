#include <gtest/gtest.h>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/threads/stw_manager.hpp>

#include <atomic>

using namespace precisegc::details::utils;
using namespace precisegc::details::threads;

TEST(managed_thread_test, test_managed_thread)
{
    auto routine = [] {
        managed_thread& mt = managed_thread::this_thread();
        managed_thread* pmt = thread_manager::instance().lookup_thread(std::this_thread::get_id());

        EXPECT_EQ(&mt, pmt);
        EXPECT_EQ(std::this_thread::get_id(), mt.get_id());
        EXPECT_EQ(pthread_self(), mt.native_handle());
    };

    scoped_thread thread(managed_thread::create(routine));
    std::thread::id thread_id = thread.get_id();

    thread.join();

    EXPECT_EQ(nullptr, thread_manager::instance().lookup_thread(thread_id));
}

TEST(managed_thread_test, test_stop_the_world)
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
        thread = managed_thread::create(routine);
    }

    while (thread_started_cnt < THREADS_CNT) {
        std::this_thread::yield();
    }

    stw_manager& stwm = stw_manager::instance();

    thread_manager::instance().stop_the_world();
    EXPECT_EQ(THREADS_CNT, stwm.threads_suspended());
    thread_manager::instance().start_the_world();

    thread_exit = true;
    guard.commit();
}

