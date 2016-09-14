#ifndef DIPLOMA_MANAGED_THREAD_HPP
#define DIPLOMA_MANAGED_THREAD_HPP

#include <thread>
#include <memory>
#include <functional>

#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>
#include <libprecisegc/details/threads/stack_map.hpp>
#include <libprecisegc/details/threads/pin_stack.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread_accessor;

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

    collectors::packet_manager::mark_packet_handle& get_mark_packet()
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
        this_managed_thread::this_thread = &this_thread;
        manager.register_thread(&this_thread);
        (*bf)();
        manager.deregister_thread(&this_thread);
    }

    managed_thread()
        : m_id(std::this_thread::get_id())
        , m_native_handle(this_thread_native_handle())
    {}

    void register_root(ptrs::gc_untyped_ptr* root)
    {
        m_root_set.insert(root);
    }

    void deregister_root(ptrs::gc_untyped_ptr* root)
    {
        m_root_set.remove(root);
    }

    size_t roots_count() const
    {
        return m_root_set.count();
    }

    template <typename Functor>
    void trace_roots(Functor&& f) const
    {
        m_root_set.trace(std::forward<Functor>(f));
    }

    void pin(byte* ptr)
    {
        m_pin_set.insert(ptr);
    }

    void unpin(byte* ptr)
    {
        m_pin_set.remove(ptr);
    }

    void push_pin(byte* ptr)
    {
        m_pin_stack.push_pin(ptr);
    }

    void pop_pin(byte* ptr)
    {
        m_pin_stack.pop_pin();
    }

    size_t pins_count() const
    {
        return m_pin_set.count() + m_pin_stack.count();
    }

    template <typename Functor>
    void trace_pins(Functor&& f) const
    {
        m_pin_set.trace(std::forward<Functor>(f));
        m_pin_stack.trace(std::forward<Functor>(f));
    }

    std::thread::native_handle_type m_native_handle;
    std::thread::id m_id;
    root_stack_map m_root_set;
    pin_stack_map m_pin_set;
    pin_stack m_pin_stack;
    collectors::packet_manager::mark_packet_handle m_mark_packet;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_HPP
