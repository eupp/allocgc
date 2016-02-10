#ifndef DIPLOMA_SIGNAL_LOCK_IMPL_H
#define DIPLOMA_SIGNAL_LOCK_IMPL_H

#include <atomic>
#include <signal.h>

namespace precisegc { namespace details {

template <typename Derived>
class signal_lock_base
{
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
        return Derived::get_sigset();
    }
};

template <typename T>
thread_local volatile sig_atomic_t signal_lock_base<T>::depth = 0;

template <typename T>
thread_local sigset_t signal_lock_base<T>::old_sigset = sigset_t();



}}

#endif //DIPLOMA_SIGNAL_LOCK_IMPL_H
