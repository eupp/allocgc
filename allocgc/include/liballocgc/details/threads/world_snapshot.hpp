#ifndef ALLOCGC_WORLD_STATE_HPP
#define ALLOCGC_WORLD_STATE_HPP

#include <mutex>
#include <utility>
#include <algorithm>

#include <liballocgc/details/threads/gc_thread_manager.hpp>
#include <liballocgc/details/threads/stw_manager.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details { namespace threads {

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

    world_snapshot(gc_thread_manager::threads_range_type threads)
        : m_threads(std::move(threads))
    {
        static stw_manager& stwm = stw_manager::instance();

        logging::info() << "Thread is requesting stop-the-world (" << std::distance(m_threads.begin(), m_threads.end()) << " running threads)";

        if (stwm.is_stop_the_world_disabled()) {
            throw stop_the_world_disabled();
        }

        for (auto& thread: m_threads) {
            if (thread->get_id() != std::this_thread::get_id()) {
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

        for (auto& thread: m_threads) {
            if (thread->get_id() != std::this_thread::get_id()) {
                stwm.resume_thread(thread->native_handle());
            }
        }
        stwm.wait_for_world_start();

        logging::info() << "World started";
    }

    void trace_roots(const gc_trace_callback& cb) const
    {
        for (auto& thread: m_threads) {
            thread->trace_roots(cb);
        }
    }

    void trace_pins(const gc_trace_pin_callback& cb) const
    {
        for (auto& thread: m_threads) {
            thread->trace_pins(cb);
        }
    }

    void trace_uninit(const gc_trace_obj_callback& cb) const
    {
        for (auto& thread: m_threads) {
            thread->trace_uninit(cb);
        }
    }

    gc_thread_descriptor* lookup_thread(std::thread::id thread_id) const
    {
        auto it = std::find_if(m_threads.begin(), m_threads.end(),
                               [thread_id] (const std::unique_ptr<gc_thread_descriptor>& thread) {
            return thread->get_id() == thread_id;
        });
        return it != m_threads.end() ? it->get() : nullptr;
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
    gc_thread_manager::threads_range_type m_threads;
    gc_clock::time_point m_time_point;
};

}}}

#endif //ALLOCGC_WORLD_STATE_HPP
