#include "gc_pause.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <atomic>
#include <type_traits>
#include <pthread.h>
#include <signal.h>

#include "thread.h"
#include "thread_list.h"
#include "signal_safe_sync.h"

namespace precisegc { namespace details {

static const int gc_signal = SIGUSR1;

static bool gc_signal_set = false;

//static thread_local bool gc_pause_disabled = false;

static signal_safe_barrier threads_paused_barrier;
static signal_safe_barrier threads_resumed_barrier;
static signal_safe_event gc_finished_event;

static size_t threads_cnt = 0;

static std::function<void(void)> gc_pause_handler;
static signal_safe_mutex gc_pause_handler_mutex;

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

class sigset_all_t {};
class sigset_gc_t {};

//template <typename Sigset>
class signal_lock_impl
{
//    template <typename S>
//    struct is_sigset_all_t: public std::is_same<S, sigset_all_t> {};
//
//    template <typename S>
//    struct is_sigset_gc_t: public std::is_same<S, sigset_gc_t> {};

//    static_assert(is_sigset_all_t<Sigset>::value || is_sigset_gc_t<Sigset>::value,
//                  "Unknown signal set");
public:
    static void lock() noexcept
    {
        if (depth == 0) {
            std::atomic_signal_fence(std::memory_order_seq_cst);
            depth = 1;
            std::atomic_signal_fence(std::memory_order_seq_cst);
            sigset_t sigset = get_sigset();
            pthread_sigmask(SIG_BLOCK, &sigset, &old_sigset);
        } else {
            depth++;
        }
    }

    static void unlock() noexcept
    {
        if (depth == 1) {
            pthread_sigmask(SIG_SETMASK, &old_sigset, nullptr);
            std::atomic_signal_fence(std::memory_order_seq_cst);
            depth = 0;
        } else {
            depth--;
        }
    }
private:
    static thread_local volatile sig_atomic_t depth;
    static thread_local sigset_t old_sigset;

    static sigset_t get_sigset() noexcept
    {
        return get_gc_sigset();
    }

//    template <typename S = Sigset>
//    static auto get_sigset()
//        -> typename std::enable_if<std::is_same<S, sigset_all_t>::value, sigset_t>::type
//    {
//        sigset_t sigset;
//        sigfillset(&sigset);
//        return sigset;
//    }

//    template <typename S = Sigset>
//    static auto get_sigset()
//        -> typename std::enable_if<std::is_same<S, sigset_gc_t>::value, sigset_t>::type
//    {
//        return get_gc_sigset();
//    }

//    template <typename S = Sigset>
//    static auto poll()
//    -> typename std::enable_if<is_sigset_gc_t<S>::value, void>::type
//    {
//        sigset_t pending;
//        int is_pending = 0;
//        do {
//            sigpending(&pending);
//            is_pending = sigismember(&pending, gc_signal);
//            if (is_pending == 1) {
//                gc_signal_handler(gc_signal);
//            }
//        } while (is_pending == 1);
//    }
};

thread_local volatile sig_atomic_t signal_lock_impl::depth = 0;
thread_local sigset_t signal_lock_impl::old_sigset = sigset_t();

void gc_pause_lock::lock() noexcept
{
    signal_lock_impl::lock();
}

void gc_pause_lock::unlock() noexcept
{
    signal_lock_impl::unlock();
}

//void signal_lock::lock() noexcept
//{
//    signal_lock_impl<sigset_all_t>::lock();
//}
//
//void signal_lock::unlock() noexcept
//{
//    signal_lock_impl<sigset_all_t>::unlock();
//}

void gc_pause()
{
//    pthread_mutex_lock(&gc_mutex);
    gc_pause_lock pause_lock;
    lock_guard<gc_pause_lock> gc_pause_guard(pause_lock);

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

    lock_guard<mutex> lock(thread_list::instance_mutex);
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
    lock_guard<signal_safe_mutex> lock(gc_pause_handler_mutex);
    gc_pause_handler = pause_handler;
}

pause_handler_t get_gc_pause_handler()
{
    lock_guard<signal_safe_mutex> lock(gc_pause_handler_mutex);
    return gc_pause_handler;
}

}}