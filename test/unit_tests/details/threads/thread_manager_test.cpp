#include <gtest/gtest.h>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
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

    thread_manager::instance().stop_the_world();
    EXPECT_EQ((size_t) THREADS_CNT, stwm.threads_suspended());
    thread_manager::instance().start_the_world();
}

TEST_F(thread_manager_test, test_get_managed_threads)
{
    auto rng = thread_manager::instance().get_managed_threads();
    std::unordered_set<std::thread::id> managed_threads;
    for (auto& mthread: rng) {
        managed_threads.insert((*mthread).get_id());
    }

    std::unordered_set<std::thread::id> all_threads;
    for (auto& thread: threads) {
        all_threads.insert(thread.get_id());
    }

    EXPECT_EQ(all_threads, managed_threads);
}