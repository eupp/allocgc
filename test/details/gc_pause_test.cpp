#include <gtest/gtest.h>

#include "pthread.h"
#include "time.h"

#include "libprecisegc/thread.h"
#include "libprecisegc/details/gc_pause.h"
#include "libprecisegc/details/condition_variable.h"
#include "libprecisegc/details/mutex.h"

using namespace precisegc::details;

size_t threads_paused_num = 0;
mutex thread_paused_mutex;
condition_variable thread_paused_cond;

size_t threads_resumed_num = 0;
mutex thread_resumed_mutex;
condition_variable thread_resumed_cond;

void pause_handler()
{
    {
        mutex_lock<mutex> lock(thread_paused_mutex);
        ++threads_paused_num;
    }
    thread_paused_cond.notify_all();
}

static void* thread_routine(void*)
{
    thread_paused_mutex.lock();
    thread_paused_cond.wait(thread_paused_mutex, [](){ return threads_paused_num == 2; });
    thread_paused_mutex.unlock();
    {
        mutex_lock<mutex> lock(thread_resumed_mutex);
        ++threads_resumed_num;
    }
    thread_resumed_cond.notify_all();
}

TEST(gc_pause_test, test_gc_pause)
{
    const int TIMEOUT = 5; // timeout in sec

    pthread_t threads[2];
    for (auto& thread: threads) {
        ASSERT_TRUE(thread_create(&thread, nullptr, thread_routine, nullptr) == 0);
    }

    gc_pause(pause_handler);

    auto get_expired_time = []() {
        timespec ts;
        ts.tv_sec = time(nullptr) + TIMEOUT;
        ts.tv_nsec = 0;
        return ts;
    };

    thread_paused_mutex.lock();
    timespec ts1 = get_expired_time();
    ASSERT_EQ(condition_variable::wait_status::no_timeout,
              thread_paused_cond.wait_for(thread_paused_mutex, &ts1, [](){ return threads_paused_num == 2; }));
    thread_paused_mutex.unlock();

    gc_resume();

    thread_resumed_mutex.lock();
    timespec ts2 = get_expired_time();
    ASSERT_EQ(condition_variable::wait_status::no_timeout,
              thread_resumed_cond.wait_for(thread_resumed_mutex, &ts2, [](){ return threads_resumed_num == 2; }));
}

