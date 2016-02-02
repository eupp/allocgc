#include <gtest/gtest.h>

#include <pthread.h>
#include <time.h>

#include "libprecisegc/details/signal_safe_sync.h"
#include "libprecisegc/details/condition_variable.h"

#include "time_util.h"

using namespace precisegc::details;

static const int TIMEOUT = 5;

static signal_safe_event g_event;

static condition_variable threads_paused_cond;
static mutex threads_paused_mutex;
static int threads_paused_num = 0;

static condition_variable threads_resumed_cond;
static mutex threads_resumed_mutex;
static int threads_resumed_num = 0;

static void* thread_routine_1(void*)
{
    {
        mutex_lock<mutex> lock(threads_paused_mutex);
        threads_paused_num++;
    }
    threads_paused_cond.notify_all();
    g_event.wait();
    {
        mutex_lock<mutex> lock(threads_resumed_mutex);
        threads_resumed_num++;
    }
    threads_resumed_cond.notify_all();
}

TEST(test_signal_safe_sync, test_event)
{
    const int THREADS_CNT = 2;
    pthread_t threads[THREADS_CNT];
    for (auto& thread: threads) {
        int res = pthread_create(&thread, nullptr, thread_routine_1, nullptr);
        ASSERT_EQ(0, res);
    }

    {
        mutex_lock<mutex> lock(threads_paused_mutex);
        timespec ts1 = ts_now();
        ts1.tv_sec += TIMEOUT;
        ASSERT_EQ(condition_variable::wait_status::no_timeout,
                  threads_paused_cond.wait_for(threads_paused_mutex, &ts1,
                                               [&threads_paused_num](){ return threads_paused_num == 2; }));
        ASSERT_EQ(0, threads_resumed_num);
    }

    g_event.notify(THREADS_CNT);

    {
        mutex_lock<mutex> lock(threads_resumed_mutex);
        timespec ts2 = ts_now();
        ts2.tv_sec += TIMEOUT;
        ASSERT_EQ(condition_variable::wait_status::no_timeout,
                  threads_resumed_cond.wait_for(threads_resumed_mutex, &ts2,
                                                [&threads_resumed_num](){ return threads_resumed_num == 2; }));
    }
}

static signal_safe_barrier g_barrier_1;

static condition_variable thread_started_cond;
static mutex thread_started_mutex;
static bool thread_started = false;

static condition_variable thread_woken_cond;
static mutex thread_woken_mutex;
static bool thread_woken = false;


static void* thread_routine_2(void*)
{
    {
        mutex_lock<mutex> lock(thread_started_mutex);
        thread_started = true;
        thread_started_cond.notify_all();
    }
    g_barrier_1.wait(1);
    {
        mutex_lock<mutex> lock(thread_woken_mutex);
        thread_woken = true;
        thread_woken_cond.notify_all();
    }
}

TEST(test_signal_safe_sync, test_barrier)
{
    pthread_t thread;
    ASSERT_EQ(0, pthread_create(&thread, nullptr, thread_routine_2, nullptr));

    {
        mutex_lock<mutex> lock(thread_started_mutex);
        timespec ts = ts_now();
        ts.tv_sec += TIMEOUT;
        ASSERT_EQ(condition_variable::wait_status::no_timeout,
                  thread_started_cond.wait_for(thread_started_mutex, &ts,
                                               [&thread_started](){ return thread_started; }));
        ASSERT_FALSE(thread_woken);
    }

    g_barrier_1.notify();

    {
        mutex_lock<mutex> lock(thread_woken_mutex);
        timespec ts = ts_now();
        ts.tv_sec += TIMEOUT;
        ASSERT_EQ(condition_variable::wait_status::no_timeout,
                  thread_woken_cond.wait_for(thread_woken_mutex, &ts,
                                             [&thread_woken](){ return thread_woken; }));
    }
}

static timeval get_timeout()
{
    const int TIMEOUT = 3;
    timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    return timeout;
}

TEST(test_signal_safe_sync, test_barrier_wait_for_1)
{
    signal_safe_barrier barrier;
    timeval timeout = get_timeout();

    ASSERT_EQ(0, barrier.wait_for(1, &timeout));
}

static signal_safe_barrier g_barrier_2;

static void* thread_routine_3(void*)
{
    g_barrier_2.notify();
}

TEST(test_signal_safe_sync, test_barrier_wait_for_2)
{
    pthread_t thread;
    ASSERT_EQ(0, pthread_create(&thread, nullptr, thread_routine_3, nullptr));

    timeval timeout = get_timeout();
    ASSERT_EQ(1, g_barrier_2.wait_for(2, &timeout));
}