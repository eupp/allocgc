#ifndef DIPLOMA_CONDITION_VARIABLE_H
#define DIPLOMA_CONDITION_VARIABLE_H

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
//#include <asm-generic/errno.h>

#include "mutex.h"

namespace precisegc { namespace details {

class condition_variable
{
public:

    enum class wait_status { timeout, no_timeout };

    condition_variable() noexcept
    {
        pthread_cond_init(&m_cond, nullptr);
    }

    ~condition_variable()
    {
        pthread_cond_destroy(&m_cond);
    }

    condition_variable(const condition_variable&) = delete;
    condition_variable& operator=(const condition_variable&) = delete;

    condition_variable(condition_variable&&) noexcept = default;
    condition_variable& operator=(condition_variable&&) noexcept = default;

    template <typename Mutex, typename Pred>
    void wait(Mutex& m, Pred pred) noexcept
    {
        while (!pred()) {
            pthread_cond_wait(&m_cond, &m.m_mutex);
        }
    }

    template <typename Mutex, typename Pred>
    wait_status wait_for(Mutex& m, const timespec* ts, Pred pred) noexcept
    {
        while (!pred()) {
            if (pthread_cond_timedwait(&m_cond, &m.m_mutex, ts) == ETIMEDOUT) {
                return wait_status::timeout;
            }
        }
        return wait_status::no_timeout;
    }

    void notify_one() noexcept
    {
        pthread_cond_signal(&m_cond);
    }

    void notify_all() noexcept
    {
        pthread_cond_broadcast(&m_cond);
    }

private:
    pthread_cond_t m_cond;
};

}}

#endif //DIPLOMA_CONDITION_VARIABLE_H
