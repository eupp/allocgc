#include <gtest/gtest.h>

#include <libprecisegc/details/threads/gc_thread_manager.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

using namespace precisegc::details::utils;
using namespace precisegc::details::threads;

//TEST(managed_thread_test, test_managed_thread)
//{
//    auto routine = [] {
//        managed_thread* this_thread = this_managed_thread::thread_ptr();
//        managed_thread* pmt = gc_thread_manager::instance().lookup_thread(std::this_thread::get_id());
//
//        EXPECT_EQ(this_thread, pmt);
//        EXPECT_EQ(std::this_thread::get_id(), this_thread->get_id());
//        EXPECT_EQ(this_thread_native_handle(), this_thread->native_handle());
//    };
//
//    scoped_thread thread(managed_thread::create(routine));
//    std::thread::id thread_id = thread.get_id();
//
//    thread.join();
//
//    EXPECT_EQ(nullptr, gc_thread_manager::instance().lookup_thread(thread_id));
//}

