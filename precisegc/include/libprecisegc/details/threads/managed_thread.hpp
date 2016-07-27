#ifndef DIPLOMA_MANAGED_THREAD_HPP
#define DIPLOMA_MANAGED_THREAD_HPP

#include <thread>
#include <memory>
#include <functional>

#include <libprecisegc/details/threads/managed_thread_accessor.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>
#include <libprecisegc/details/threads/stack_map.hpp>
#include <libprecisegc/details/threads/tlab.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread : private utils::noncopyable, private utils::nonmovable
{
public:
    static managed_thread& main_thread()
    {
        static managed_thread thread;
        return thread;
    }

    template <typename Function, typename... Args>
    static std::thread create(Function&& f, Args&&... args)
    {
        typedef decltype(std::bind(std::forward<Function>(f), std::forward<Args>(args)...)) functor_type;
        std::unique_ptr<functor_type> pf(
                new functor_type(std::bind(std::forward<Function>(f), std::forward<Args>(args)...))
        );
        return std::thread(&start_routine<functor_type>, std::move(pf));
    };

    std::thread::id get_id() const
    {
        return m_id;
    }

    std::thread::native_handle_type native_handle() const
    {
        return m_native_handle;
    }

    std::unique_ptr<collectors::mark_packet>& get_mark_packet()
    {
        return m_mark_packet;
    }

    friend class managed_thread_accessor;
private:
    template <typename Functor>
    static void start_routine(std::unique_ptr<Functor> bf)
    {
        static thread_manager& manager = thread_manager::instance();

        managed_thread this_thread;
        managed_thread_accessor::set_this_managed_thread_pointer(&this_thread);
        manager.register_thread(&this_thread);
        (*bf)();
        manager.deregister_thread(&this_thread);
    }

    managed_thread()
        : m_id(std::this_thread::get_id())
        , m_native_handle(this_thread_native_handle())
    {}

    std::thread::native_handle_type m_native_handle;
    std::thread::id m_id;
    root_stack_map m_root_set;
    pin_stack_map m_pin_set;
    tlab m_tlab;
    std::unique_ptr<collectors::mark_packet> m_mark_packet;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_HPP
