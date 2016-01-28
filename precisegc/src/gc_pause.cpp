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

static signal_safe_barrier threads_paused_barrier;
static signal_safe_barrier threads_resumed_barrier;
static signal_safe_event gc_finished_event;

static size_t threads_cnt = 0;

static std::function<void(void)> gc_pause_handler;
static mutex gc_pause_handler_mutex;

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

void gc_pause()
{
//    pthread_mutex_lock(&gc_mutex);

    if (!gc_signal_set) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, gc_signal);
        struct sigaction sa;
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = gc_signal_handler;
        sa.sa_mask = sigset;
        int sa_ret = sigaction(gc_signal, &sa, nullptr);
        assert(sa_ret == 0);
        gc_signal_set = true;
    }

    thread_list::locked_instance tl_instance;
    for (auto it = tl_instance->begin(); it != tl_instance->end(); ++it) {
        if (!pthread_equal(it->thread, pthread_self())) {
            pthread_kill(it->thread, gc_signal);
        }
    }

    threads_cnt = tl_instance->size() - 1;
    threads_paused_barrier.wait(threads_cnt);
}

void gc_resume()
{
    gc_finished_event.notify(threads_cnt);
    threads_resumed_barrier.wait(threads_cnt);
//    pthread_mutex_unlock(&gc_mutex);
}

void set_gc_pause_handler(const std::function<void(void)>& pause_handler)
{
    mutex_lock<mutex> lock(gc_pause_handler_mutex);
    gc_pause_handler = pause_handler;
}

std::function<void(void)> get_gc_pause_handler()
{
    mutex_lock<mutex> lock(gc_pause_handler_mutex);
    return gc_pause_handler;
}

}}

