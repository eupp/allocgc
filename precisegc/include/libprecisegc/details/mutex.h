#ifndef DIPLOMA_MUTEX_H
#define DIPLOMA_MUTEX_H

#include <pthread.h>

namespace precisegc { namespace details {

class condition_variable;

class mutex
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

    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

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

    friend class condition_variable;
private:
    pthread_mutex_t m_mutex;
};

class recursive_mutex
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

    recursive_mutex(const recursive_mutex& other) = delete;
    recursive_mutex& operator=(const recursive_mutex& other) = delete;

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
class mutex_lock
{
public:

    mutex_lock(Mutex& m)
        : m_mutex(m)
    {
        m_mutex.lock();
    }

    ~mutex_lock()
    {
        m_mutex.unlock();
    }

private:
    Mutex& m_mutex;
};

} }

#endif //DIPLOMA_MUTEX_H
