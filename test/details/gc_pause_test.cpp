#include <gtest/gtest.h>

#include <atomic>

#include <pthread.h>
#include <time.h>

#include "libprecisegc/thread.h"
#include "libprecisegc/details/gc_pause.h"
#include "libprecisegc/details/signal_safe_sync.h"

using namespace precisegc;
using namespace precisegc::details;

std::atomic<size_t> threads_paused_num(0);
std::atomic<size_t> threads_resumed_num(0);

signal_safe_barrier threads_paused_barrier;
signal_safe_barrier threads_resumed_barrier;

signal_safe_event threads_resumed_event;

//size_t threads_paused_num = 0;
//mutex thread_paused_mutex;
//condition_variable thread_paused_cond;
//
//size_t threads_resumed_num = 0;
//mutex thread_resumed_mutex;
//condition_variable thread_resumed_cond;

void pause_handler()
{
    threads_paused_num++;
    threads_paused_barrier.notify();
}

static void* thread_routine(void*)
{
    threads_resumed_event.wait();
    threads_resumed_num++;
    threads_resumed_barrier.notify();
}

TEST(gc_pause_test, test_gc_pause)
{
    const int TIMEOUT = 5; // timeout in sec
    const int THREADS_CNT = 2;

    pthread_t threads[THREADS_CNT];
    for (auto& thread: threads) {
        ASSERT_TRUE(thread_create(&thread, nullptr, thread_routine, nullptr) == 0);
    }

    set_gc_pause_handler(pause_handler);
    gc_pause();

    threads_paused_barrier.wait(THREADS_CNT);
    ASSERT_EQ(THREADS_CNT, threads_paused_num);
    ASSERT_EQ(0, threads_resumed_num);

    threads_resumed_event.notify(THREADS_CNT);
    gc_resume();

    threads_resumed_barrier.wait(THREADS_CNT);
    ASSERT_EQ(THREADS_CNT, threads_resumed_num);
}

