#ifndef DIPLOMA_MUTEX_H
#define DIPLOMA_MUTEX_H

#include <pthread.h>

namespace precisegc { namespace details {

class mutex
{
public:

    mutex()
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

private:
    pthread_mutex_t m_mutex;
};

class recursive_mutex
{
public:

    recursive_mutex()
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

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
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

    mutex_lock(Mutex* m)
        : m_mutex(m)
    {
        m_mutex->lock();
    }

    ~mutex_lock()
    {
        m_mutex->unlock();
    }

private:
    Mutex* m_mutex;
};

} }

#endif //DIPLOMA_MUTEX_H
