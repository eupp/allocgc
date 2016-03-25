#include "gc_pause.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <atomic>
#include <type_traits>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "thread.h"
#include "thread_list.h"
#include "signal_safe_sync.h"
#include "logging.h"

namespace precisegc { namespace details {

static bool gc_signal_set = false;

static thread_local bool gc_pause_disabled = false;

static gc_signal_safe_mutex gc_mutex;

static signal_safe_barrier threads_paused_barrier;
static signal_safe_barrier threads_resumed_barrier;
static signal_safe_event gc_finished_event;

static size_t threads_cnt = 0;

static std::function<void(void)> gc_pause_handler;
static gc_signal_safe_mutex gc_pause_handler_mutex;

static void gc_signal_handler()
{
    threads_paused_barrier.notify();
    if (gc_pause_handler) {
        gc_pause_handler();
    }
    gc_finished_event.wait();
    threads_resumed_barrier.notify();
}

static void check_gc_siglock(int signum)
{
    assert(signum == gc_signal);

    logging::info() << "Thread " << pthread_self() << " enters gc signal handler";

    if (flag_gc_signal_lock::is_locked()) {
        flag_gc_signal_lock::set_pending();
        return;
    }

    gc_signal_handler();
}

gc_pause_disabled_exception::gc_pause_disabled_exception()
    : m_msg("gc_pause() is called by thread " + std::to_string(pthread_self()) + " which disable it")
{}

const char* gc_pause_disabled_exception::what() const noexcept
{
    return m_msg.c_str();
}

thread_local volatile sig_atomic_t flag_gc_signal_lock::depth = 0;
thread_local volatile sig_atomic_t flag_gc_signal_lock::signal_pending_flag = false;

int flag_gc_signal_lock::lock() noexcept
{
    if (depth == 0) {
        std::atomic_signal_fence(std::memory_order_seq_cst);
        depth = 1;
        std::atomic_signal_fence(std::memory_order_seq_cst);
    } else {
        depth++;
    }
    return depth;
}

int flag_gc_signal_lock::unlock() noexcept
{
    if (depth == 1) {
        if (signal_pending_flag) {
            gc_signal_handler();
            signal_pending_flag = false;
        }
        std::atomic_signal_fence(std::memory_order_seq_cst);
        depth = 0;
    } else {
        depth--;
    }
    return depth;
}

bool flag_gc_signal_lock::is_locked() noexcept
{
    return depth > 0;
}

void flag_gc_signal_lock::set_pending() noexcept
{
    signal_pending_flag = true;
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

void gc_pause_lock::lock() noexcept
{
    siglock::lock();
    gc_pause_disabled = true;
}

void gc_pause_lock::unlock() noexcept
{
    if (!siglock::unlock()) {
        gc_pause_disabled = false;
    }
}

void gc_pause_init()
{
    if (!gc_signal_set) {
        sigset_t sigset = get_gc_sigset();
        struct sigaction sa;
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = check_gc_siglock;
        sa.sa_mask = sigset;
        int sa_ret = sigaction(gc_signal, &sa, nullptr);
        assert(sa_ret == 0);
        gc_signal_set = true;
    }
}

void gc_pause()
{
    assert(gc_signal_set);

    if (gc_pause_disabled) {
        throw gc_pause_disabled_exception();
    }

    while (!gc_mutex.try_lock()) {
        // very bad code, we need better wait strategy
        sleep(1);
    }

    logging::info() << "Thread " << pthread_self() << " is requesting stop-the-world";

    {
        lock_guard<mutex> lock(thread_list::instance_mutex);
        thread_list& threads = thread_list::instance();
        threads_cnt = threads.size() - 1;
        pthread_t self = pthread_self();

        for (auto& thread: threads) {
            if (!pthread_equal(thread.pthread, self)) {
                logging::info() << "Sending gc_signal to thread " << thread.pthread;
                pthread_kill(thread.pthread, gc_signal);
            }
        }
    }

//    logging::info() << "GC signal broadcastet";

    threads_paused_barrier.wait(threads_cnt);

    logging::info() << "All threads are suspended";
}

void gc_resume()
{
    logging::info() << "Thread " << pthread_self() << " is requesting start-the-world";

    gc_finished_event.notify(threads_cnt);
    threads_resumed_barrier.wait(threads_cnt);
    gc_mutex.unlock();
}

void set_gc_pause_handler(const pause_handler_t& pause_handler)
{
    lock_guard<gc_signal_safe_mutex> lock(gc_pause_handler_mutex);
    gc_pause_handler = pause_handler;
}

pause_handler_t get_gc_pause_handler()
{
    lock_guard<gc_signal_safe_mutex> lock(gc_pause_handler_mutex);
    return gc_pause_handler;
}
}}