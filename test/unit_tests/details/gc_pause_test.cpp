#include <gtest/gtest.h>

#include <atomic>

#include <pthread.h>
#include <time.h>

#include "libprecisegc/thread.h"
#include "libprecisegc/details/gc_pause.h"
#include "libprecisegc/details/signal_safe_sync.h"
#include "libprecisegc/details/logging.h"

using namespace precisegc;
using namespace precisegc::details;

std::atomic<size_t> threads_paused_num(0);
std::atomic<size_t> threads_resumed_num(0);

signal_safe_barrier threads_paused_barrier;
signal_safe_barrier threads_resumed_barrier;

signal_safe_event threads_resumed_event;

static void* thread_routine_1(void*)
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
        ASSERT_EQ(0, thread_create(&thread, nullptr, thread_routine_1, nullptr));
    }

    pause_handler_setter handler_setter([&threads_paused_num, &threads_paused_barrier]() {
        threads_paused_num++;
        threads_paused_barrier.notify();
    });
    gc_pause();

    threads_paused_barrier.wait(THREADS_CNT);
    ASSERT_EQ(THREADS_CNT, threads_paused_num);
    ASSERT_EQ(0, threads_resumed_num);

    threads_resumed_event.notify(THREADS_CNT);
    gc_resume();

    threads_resumed_barrier.wait(THREADS_CNT);
    ASSERT_EQ(THREADS_CNT, threads_resumed_num);
}

TEST(gc_pause_test, test_gc_pause_disabled_1)
{
    gc_pause_lock pause_lock;
    lock_guard<gc_pause_lock> lock(pause_lock);
    ASSERT_THROW(gc_pause(), gc_pause_disabled_exception);
}

static std::atomic<size_t> g_counter(0);
static std::atomic<bool> gc_pause_enabled(false);

static void* thread_routine_2(void*)
{
    while (!gc_pause_enabled);
}

static void* thread_routine_3(void*)
{
//    std::cout << "Thread " << pthread_self() << " gc" << std::endl;
    gc_pause();
    gc_resume();
}

#include <iostream>

TEST(gc_pause_test, test_gc_pause_lock_2)
{
    logging::init(std::cout, logging::loglevel::INFO);

    const int THREADS_CNT = 10;

//    std::cout << "Thread " << pthread_self() << " test." << std::endl;
    pthread_t threads[THREADS_CNT];
    for (auto& thread: threads) {
        ASSERT_EQ(0, thread_create(&thread, nullptr, thread_routine_2, nullptr));
//        std::cout << "Thread " << thread << " worker." << std::endl;
    }

    pause_handler_setter handler_setter([&g_counter]() {
        ++g_counter;
//        std::cout << "Thread " << pthread_self() << " reach signal handler." << std::endl;
    });

    {
        gc_pause_lock pause_lock;
        lock_guard<gc_pause_lock> lock(pause_lock);

        pthread_t gc_thread;
        ASSERT_EQ(0, thread_create(&gc_thread, nullptr, thread_routine_3, nullptr));

        sleep(1);
        // Assert that all other threads, except us, increase counter.
        // If disable_gc_pause is broken then likely this thread have been interrupted by pause handler before this assertion,
        // and counter had been incremented to THREADS_CNT + 1.
        ASSERT_EQ(THREADS_CNT, g_counter);
    }

    gc_pause_enabled = true;
    sleep(1);

    ASSERT_EQ(THREADS_CNT + 1, g_counter);
}

