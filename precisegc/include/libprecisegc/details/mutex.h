#ifndef DIPLOMA_MUTEX_H
#define DIPLOMA_MUTEX_H

#include <pthread.h>

#include "util.h"

namespace precisegc { namespace details {

class condition_variable;

class mutex: public noncopyable
{
public:

    mutex() noexcept
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    mutex(mutex&&) noexcept = default;
    mutex& operator=(mutex&&) noexcept = default;

    void lock() noexcept
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() noexcept
    {
        pthread_mutex_unlock(&m_mutex);
    }

    bool try_lock() noexcept
    {
        return pthread_mutex_trylock(&m_mutex) == 0;
    }

    friend class condition_variable;
private:
    pthread_mutex_t m_mutex;
};

class recursive_mutex: public noncopyable
{
public:

    recursive_mutex() noexcept
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    ~recursive_mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    recursive_mutex(recursive_mutex&&) noexcept = default;
    recursive_mutex& operator=(recursive_mutex&&) noexcept = default;

    void lock() noexcept
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() noexcept
    {
        pthread_mutex_unlock(&m_mutex);
    }

private:
    pthread_mutex_t m_mutex;
};

template <typename Mutex>
class lock_guard: public noncopyable
{
public:

    lock_guard(Mutex& m)
        : m_mutex(m)
    {
        m_mutex.lock();
    }

    ~lock_guard()
    {
        m_mutex.unlock();
    }

private:
    Mutex& m_mutex;
};

} }

#endif //DIPLOMA_MUTEX_H
