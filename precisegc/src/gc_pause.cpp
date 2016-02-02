#include "gc_pause.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <pthread.h>
#include <signal.h>

#include "thread.h"
#include "thread_list.h"
#include "signal_safe_sync.h"

namespace precisegc { namespace details {

static const int gc_signal = SIGUSR1;

static bool gc_signal_set = false;

static thread_local bool gc_pause_disabled = false;

static signal_safe_barrier threads_paused_barrier;
static signal_safe_barrier threads_resumed_barrier;
static signal_safe_event gc_finished_event;

static size_t threads_cnt = 0;

static std::function<void(void)> gc_pause_handler;
static mutex gc_pause_handler_mutex;

static sigset_t get_gc_sigset()
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, gc_signal);
    return sigset;
}

static void gc_signal_handler(int signum)
{
    assert(signum == gc_signal);
    threads_paused_barrier.notify();
    if (gc_pause_handler) {
        gc_pause_handler();
    }
    gc_finished_event.wait();
    threads_resumed_barrier.notify();
}

gc_pause_disabled_exception::gc_pause_disabled_exception()
    : m_msg("gc_pause() is called by thread " + std::to_string(pthread_self()) + " which disable it")
{}

const char* gc_pause_disabled_exception::what() const noexcept
{
    return m_msg.c_str();
}

void enable_gc_pause()
{
    sigset_t sigset = get_gc_sigset();
    pthread_sigmask(SIG_UNBLOCK, &sigset, nullptr);
    gc_pause_disabled = false;

    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    int res = sigtimedwait(&sigset, nullptr, &ts);
    if (res == gc_signal) {
        gc_signal_handler(gc_signal);
    }
}

void disable_gc_pause()
{
    sigset_t sigset = get_gc_sigset();
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);
    gc_pause_disabled = true;
}

void gc_pause()
{
//    pthread_mutex_lock(&gc_mutex);

    if (gc_pause_disabled) {
        throw gc_pause_disabled_exception();
    }

    if (!gc_signal_set) {
        sigset_t sigset = get_gc_sigset();
        struct sigaction sa;
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = gc_signal_handler;
        sa.sa_mask = sigset;
        int sa_ret = sigaction(gc_signal, &sa, nullptr);
        assert(sa_ret == 0);
        gc_signal_set = true;
    }

    mutex_lock<mutex> lock(thread_list::instance_mutex);
    thread_list& threads = thread_list::instance();
    threads_cnt = threads.size() - 1;
    pthread_t self = pthread_self();

    for (auto& thread: threads) {
        if (!pthread_equal(thread.pthread, self)) {
            pthread_kill(thread.pthread, gc_signal);
        }
    }

    threads_paused_barrier.wait(threads_cnt);
}

void gc_resume()
{
    gc_finished_event.notify(threads_cnt);
    threads_resumed_barrier.wait(threads_cnt);
//    pthread_mutex_unlock(&gc_mutex);
}

void set_gc_pause_handler(const pause_handler_t& pause_handler)
{
    mutex_lock<mutex> lock(gc_pause_handler_mutex);
    gc_pause_handler = pause_handler;
}

pause_handler_t get_gc_pause_handler()
{
    mutex_lock<mutex> lock(gc_pause_handler_mutex);
    return gc_pause_handler;
}

}}

