#include <gtest/gtest.h>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/threads/stw_manager.hpp>

#include <atomic>
#include <unordered_set>

using namespace precisegc::details::utils;
using namespace precisegc::details::threads;

class thread_manager_test : public ::testing::Test
{
public:
    thread_manager_test()
        : thread_started_cnt(0)
        , thread_exit(false)
    {
        auto routine = [this] {
            ++thread_started_cnt;
            while (!thread_exit) {};
        };
        for (auto& thread: threads) {
            thread = managed_thread::create(routine);
        }
        while (thread_started_cnt < THREADS_CNT) {
            std::this_thread::yield();
        }
    }

    ~thread_manager_test()
    {
        thread_exit = true;
    }

    static const size_t THREADS_CNT = 10;

    scoped_thread threads[THREADS_CNT];
    std::atomic<size_t> thread_started_cnt;
    std::atomic<bool> thread_exit;
};

TEST_F(thread_manager_test, test_stop_the_world)
{
    stw_manager& stwm = stw_manager::instance();

    {
        world_snapshot snapshot = thread_manager::instance().stop_the_world();
        EXPECT_EQ((size_t) THREADS_CNT, stwm.threads_suspended());
    }
    EXPECT_EQ(0, stwm.threads_suspended());
}

TEST_F(thread_manager_test, test_get_managed_threads)
{
    using namespace precisegc::details::threads::internals;

    std::unordered_set<std::thread::id> managed_threads;
    for (auto thread: thread_manager::instance().threads_snapshot()) {
        managed_threads.insert((*thread).get_id());
    }

    std::unordered_set<std::thread::id> all_threads;
    for (auto& thread: threads) {
        all_threads.insert(thread.get_id());
    }
    all_threads.insert(std::this_thread::get_id());

    EXPECT_EQ(all_threads, managed_threads);
}
