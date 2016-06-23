#ifndef DIPLOMA_THREADS_SNAPSHOT_HPP
#define DIPLOMA_THREADS_SNAPSHOT_HPP

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class threads_snapshot : private utils::noncopyable
{
    typedef internals::thread_manager_access::range_type range_type;
public:
    threads_snapshot(range_type threads)
        : m_threads(std::move(threads))
    {}

    threads_snapshot(threads_snapshot&& other) = default;

    template <typename Functor>
    void trace_barrier_buffers(Functor&& f) const
    {
        if (m_threads.owns_lock()) {
            for (auto thread: m_threads) {
                thread->get_barrier_buffer().trace(f);
            }
        }
    }
private:
    range_type m_threads;
};

}}}

#endif //DIPLOMA_THREADS_SNAPSHOT_HPP
