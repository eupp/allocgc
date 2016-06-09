#ifndef DIPLOMA_MANAGED_THREAD_HPP
#define DIPLOMA_MANAGED_THREAD_HPP

#include <thread>
#include <functional>
#include <memory>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/stack_map.hpp>
#include <libprecisegc/details/barrier_buffer.h>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread : private utils::noncopyable, private utils::nonmovable
{
public:
    static managed_thread& this_thread()
    {
        static thread_local managed_thread mt;
        return mt;
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

    root_stack_map& root_set()
    {
        return m_root_set;
    }

    pin_stack_map& pin_set()
    {
        return m_pin_set;
    }

    barrier_buffer& get_barrier_buffer()
    {
        return m_barrier_buf;
    }
private:
    template <typename Functor>
    static void start_routine(std::unique_ptr<Functor>&& bf)
    {
        static thread_manager& manager = thread_manager::instance();

        managed_thread& mt = this_thread();
        manager.register_thread(&mt);
        (*bf)();
        manager.deregister_thread(&mt);
    }

    managed_thread()
        : m_id(std::this_thread::get_id())
        , m_native_handle(this_thread_native_handle())
    {}

    std::thread::native_handle_type m_native_handle;
    std::thread::id m_id;
    root_stack_map m_root_set;
    pin_stack_map m_pin_set;
    barrier_buffer m_barrier_buf;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_HPP
