#ifndef DIPLOMA_WORLD_STATE_HPP
#define DIPLOMA_WORLD_STATE_HPP

#include <mutex>
#include <utility>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/stw_manager.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details { namespace threads {

class world_state
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

    world_state(range_type&& threads)
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

        logging::info() << "World stopped";
    }

    world_state(world_state&& other)
        : m_threads(std::move(other.m_threads))
    {}

    ~world_state()
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
            root_set::element* it = thread->get_root_set().head();
            while (it != nullptr) {
                f(it->root->get());
                it = it->next;
            }
        }
    }

    template <typename Functor>
    void trace_roots(const Functor& f) const
    {
        for (auto thread: m_threads) {
            root_set::element* it = thread->get_root_set().head();
            while (it != nullptr) {
                f(it->root->get());
                it = it->next;
            }
        }
    }

    template <typename Functor>
    void trace_pins(Functor& f) const
    {
        for (auto thread: m_threads) {
            root_set::element* it = thread->get_pin_set().head();
            while (it != nullptr) {
                f(it->root->get());
                it = it->next;
            }
        }
    }

    template <typename Functor>
    void trace_pins(const Functor& f) const
    {
        for (auto thread: m_threads) {
            root_set::element* it = thread->get_pin_set().head();
            while (it != nullptr) {
                f(it->root->get());
                it = it->next;
            }
        }
    }
private:
    range_type m_threads;
};

}}}

#endif //DIPLOMA_WORLD_STATE_HPP
