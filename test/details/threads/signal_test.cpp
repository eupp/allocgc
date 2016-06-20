#include <gtest/gtest.h>

#include <libprecisegc/details/threads/posix_signal.hpp>

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

using namespace precisegc::details::threads;

namespace {
int counter = 0;

void sighandler()
{
    ++counter;
}
}

class posix_signal_test : public ::testing::Test
{
public:
    posix_signal_test()
    {
        counter = 0;
        posix_signal& sig = posix_signal::instance();
        old_handler = sig.get_handler();
        sig.set_handler(sighandler);
    }

    ~posix_signal_test()
    {
        posix_signal& sig = posix_signal::instance();
        sig.set_handler(old_handler);
    }

    posix_signal::handler_type old_handler;
};

TEST_F(posix_signal_test, test_send)
{
    std::atomic<bool> thread_started(false);
    std::atomic<bool> thread_exit(false);
    auto routine = [&thread_started, &thread_exit] {
        thread_started = true;
        while (!thread_exit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    auto thread = std::thread(routine);
    while (!thread_started) {
        std::this_thread::yield();
    }

    EXPECT_EQ(0, counter);

    posix_signal::instance().send(thread.native_handle());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ(1, counter);

    thread_exit = true;
    thread.join();
}

TEST_F(posix_signal_test, test_lock)
{
    std::atomic<bool> thread_started(false);
    std::atomic<bool> thread_exit(false);
    auto routine = [&thread_started, &thread_exit] {
        thread_started = true;
        std::lock_guard<posix_signal> lock(posix_signal::instance());
        while (!thread_exit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    auto thread = std::thread(routine);
    while (!thread_started) {
        std::this_thread::yield();
    }

    EXPECT_EQ(0, counter);

    posix_signal::instance().send(thread.native_handle());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ(0, counter);

    thread_exit = true;
    thread.join();

    EXPECT_EQ(1, counter);
}

