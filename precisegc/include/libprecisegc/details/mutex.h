#ifndef DIPLOMA_MUTEX_H
#define DIPLOMA_MUTEX_H

#include <pthread.h>

#include "util.h"

namespace precisegc { namespace details {

class condition_variable;

class mutex : public noncopyable, public nonmovable
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

class recursive_mutex : public noncopyable, public nonmovable
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
        pthread_mutex_destroy(& m_mutex);
    }

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

class shared_mutex : public noncopyable, public nonmovable
{
public:
    shared_mutex() noexcept
    {
        pthread_rwlock_init(&m_mutex, nullptr);
    }

    ~shared_mutex() noexcept
    {
        pthread_rwlock_destroy(&m_mutex);
    }

    void lock() noexcept
    {
        pthread_rwlock_wrlock(&m_mutex);
    }

    bool try_lock() noexcept
    {
        return pthread_rwlock_trywrlock(&m_mutex) == 0;
    }

    void unlock() noexcept
    {
        pthread_rwlock_unlock(&m_mutex);
    }

    void lock_shared() noexcept
    {
        pthread_rwlock_rdlock(&m_mutex);
    }

    bool try_lock_shared() noexcept
    {
        return pthread_rwlock_tryrdlock(&m_mutex) == 0;
    }

    void unlock_shared() noexcept
    {
        pthread_rwlock_unlock(&m_mutex);
    }
private:
    pthread_rwlock_t m_mutex;
};

template <typename Mutex>
class lock_guard : public noncopyable, public nonmovable
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

template <typename Mutex>
class shared_lock_guard : public noncopyable, public nonmovable
{
public:
    shared_lock_guard(Mutex& m)
        : m_mutex(m)
    {
        m_mutex.lock_shared();
    }

    ~shared_lock_guard()
    {
        m_mutex.unlock_shared();
    }
private:
    Mutex& m_mutex;
};

} }

#endif //DIPLOMA_MUTEX_H
