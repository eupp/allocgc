#ifndef DIPLOMA_WORLD_STATE_HPP
#define DIPLOMA_WORLD_STATE_HPP

#include <mutex>
#include <utility>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/stw_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_clock.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details { namespace threads {

class world_snapshot : private utils::noncopyable
{
    typedef internals::thread_manager_access::range_type range_type;
public:
    class stop_the_world_disabled : public gc_exception
    {
    public:
        stop_the_world_disabled()
            : gc_exception("stop-the-world is disabled by current thread")
        {}
    };

    world_snapshot(range_type&& threads)
        : m_threads(std::move(threads))
    {
        static stw_manager& stwm = stw_manager::instance();

        logging::info() << "Thread " << std::this_thread::get_id() << " is requesting stop-the-world";

        if (stwm.is_stop_the_world_disabled()) {
            throw stop_the_world_disabled();
        }

        for (auto thread: m_threads) {
            if (thread->native_handle() != managed_thread::this_thread().native_handle()) {
                stwm.suspend_thread(thread->native_handle());
            }
        }
        stwm.wait_for_world_stop();

        m_time_point = gc_clock::now();

        logging::info() << "World stopped";
    }

    world_snapshot(world_snapshot&& other)
        : m_threads(std::move(other.m_threads))
    {}

    ~world_snapshot()
    {
        static stw_manager& stwm = stw_manager::instance();

        logging::info() << "Thread " << std::this_thread::get_id() << " is requesting start-the-world";

        for (auto thread: m_threads) {
            if (thread->native_handle() != managed_thread::this_thread().native_handle()) {
                stwm.resume_thread(thread->native_handle());
            }
        }
        stwm.wait_for_world_start();

        logging::info() << "World started";
    }

    template <typename Functor>
    void trace_roots(Functor& f) const
    {
        for (auto thread: m_threads) {
            thread->root_set().shrink();
            thread->root_set().trace(f);
        }
    }

    template <typename Functor>
    void trace_roots(const Functor& f) const
    {
        for (auto thread: m_threads) {
            thread->root_set().shrink();
            for (auto thread: m_threads) {
                thread->root_set().trace(f);
            }
        }
    }

    template <typename Functor>
    void trace_pins(Functor& f) const
    {
        for (auto thread: m_threads) {
            thread->root_set().shrink();
            for (auto thread: m_threads) {
                thread->pin_set().trace(f);
            }
        }
    }

    template <typename Functor>
    void trace_pins(const Functor& f) const
    {
        for (auto thread: m_threads) {
            thread->root_set().shrink();
            for (auto thread: m_threads) {
                thread->pin_set().trace(f);
            }
        }
    }

    gc_clock::time_point time_point() const
    {
        return m_time_point;
    }

    gc_clock::duration time_since_stop_the_world() const
    {
        return gc_clock::now() - m_time_point;
    }
private:
    range_type m_threads;
    gc_clock::time_point m_time_point;
};

}}}

#endif //DIPLOMA_WORLD_STATE_HPP
