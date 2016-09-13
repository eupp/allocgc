#ifndef DIPLOMA_WORLD_STATE_HPP
#define DIPLOMA_WORLD_STATE_HPP

#include <mutex>
#include <utility>
#include <algorithm>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/managed_thread_accessor.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/stw_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_clock.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/details/forwarding.h>

namespace precisegc { namespace details { namespace threads {

class world_snapshot : private utils::noncopyable
{
public:
    class stop_the_world_disabled : public gc_exception
    {
    public:
        stop_the_world_disabled()
            : gc_exception("stop-the-world is disabled by current thread")
        {}
    };

    class root_tracer
    {
    public:
        template <typename Functor>
        void trace(Functor&& f) const
        {
            m_snapshot.trace_roots(std::forward<Functor>(f));
        }

        friend class world_snapshot;
    private:
        root_tracer(const world_snapshot& snapshot)
            : m_snapshot(snapshot)
        {}

        const world_snapshot& m_snapshot;
    };

    class pin_tracer
    {
    public:
        template <typename Functor>
        void trace(Functor&& f) const
        {
            m_snapshot.trace_pins(std::forward<Functor>(f));
        }

        friend class world_snapshot;
    private:
        pin_tracer(const world_snapshot& snapshot)
            : m_snapshot(snapshot)
        {}

        const world_snapshot& m_snapshot;
    };

    world_snapshot(thread_manager::threads_range_type threads)
        : m_threads(std::move(threads))
    {
        static stw_manager& stwm = stw_manager::instance();

        logging::info() << "Thread is requesting stop-the-world (" << std::distance(m_threads.begin(), m_threads.end()) << " running threads)";

        if (stwm.is_stop_the_world_disabled()) {
            throw stop_the_world_disabled();
        }

        for (auto thread: m_threads) {
            if (thread->get_id() != this_managed_thread::get_id()) {
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

        logging::info() << "Thread is requesting start-the-world";

        for (auto thread: m_threads) {
            if (thread->get_id() != this_managed_thread::get_id()) {
                stwm.resume_thread(thread->native_handle());
            }
        }
        stwm.wait_for_world_start();

        logging::info() << "World started";
    }

    root_tracer get_root_tracer() const
    {
        return root_tracer(*this);
    }

    pin_tracer get_pin_tracer() const
    {
        return pin_tracer(*this);
    }

    template <typename Functor>
    void trace_roots(Functor&& f) const
    {
        for (auto thread: m_threads) {
            auto& rs = managed_thread_accessor::get_root_set(thread);
            logging::info() << "Thread " << thread->get_id() << " roots count = " << rs.count();
            rs.trace(std::forward<Functor>(f));
        }
    }

    template <typename Functor>
    void trace_pins(Functor&& f) const
    {
        for (auto thread: m_threads) {
            auto& pset = managed_thread_accessor::get_pin_set(thread);
            auto& pstack = managed_thread_accessor::get_pin_stack(thread);
            logging::info() << "Thread " << thread->get_id()
                             << " pins count = " << pset.count() + pstack.count();
            pset.trace(std::forward<Functor>(f));
            pstack.trace(std::forward<Functor>(f));
        }
    }

    template <typename Forwarding>
    void fix_roots(const Forwarding& forwarding) const
    {
        logging::info() << "Fixing roots...";
        for (auto thread: m_threads) {
            managed_thread_accessor::get_root_set(thread).trace([&forwarding] (ptrs::gc_untyped_ptr* p) {
                forwarding.forward(p);
            });
        }
    }

    template <typename Functor>
    void apply_to_threads(Functor&& f) const
    {
        for (auto thread: m_threads) {
            f(thread);
        }
    }

    managed_thread* lookup_thread(std::thread::id thread_id) const
    {
        auto it = std::find_if(m_threads.begin(), m_threads.end(), [thread_id] (managed_thread* thread) {
            return thread->get_id() == thread_id;
        });
        return it != m_threads.end() ? *it : nullptr;
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
    thread_manager::threads_range_type m_threads;
    gc_clock::time_point m_time_point;
};

}}}

#endif //DIPLOMA_WORLD_STATE_HPP
