#ifndef DIPLOMA_BARRIER_HPP
#define DIPLOMA_BARRIER_HPP

#include <pthread.h>

namespace precisegc { namespace details { namespace utils {

class barrier
{
public:

    barrier(size_t thread_count) noexcept
    {
        pthread_barrier_init(&m_barrier, nullptr, thread_count);
    }

    ~barrier()
    {
        pthread_barrier_destroy(&m_barrier);
    }

    barrier(const barrier&) = delete;
    barrier& operator=(const barrier&) = delete;

    barrier(barrier&&) noexcept = default;
    barrier& operator=(barrier&&) noexcept = default;

    void wait()
    {
        pthread_barrier_wait(&m_barrier);
    }

private:
    pthread_barrier_t m_barrier;
};

}}}

#endif //DIPLOMA_BARRIER_HPP
