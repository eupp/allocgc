#ifndef DIPLOMA_BARRIER_HPP
#define DIPLOMA_BARRIER_HPP

#include <mutex>
#include <condition_variable>

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace utils {

class barrier : private noncopyable, private nonmovable
{
public:
    barrier(size_t thread_count)
        : m_threads_count(thread_count)
        , m_threads_waiting(0)
    {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        ++m_threads_waiting;
        if (m_threads_waiting < m_threads_count) {
            m_condvar.wait(lock, [this] { return m_threads_waiting == 0; });
        } else {
            m_threads_waiting = 0;
            m_condvar.notify_all();
        }
    }
private:
    const size_t m_threads_count;
    size_t m_threads_waiting;
    std::mutex m_mutex;
    std::condition_variable m_condvar;
};

}}}

#endif //DIPLOMA_BARRIER_HPP
