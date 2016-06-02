#include <gtest/gtest.h>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/threads/stw_manager.hpp>

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

