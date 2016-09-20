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
#include <libprecisegc/details/threads/gc_new_stack.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread_accessor;

class managed_thread : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef gc_new_stack::offsets_range gc_ptr_offsets_range;

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

    bool is_heap_ptr(byte* ptr) const
    {
        return m_new_stack.is_heap_ptr(ptr);
    }

    bool is_type_meta_requested() const
    {
        return m_new_stack.is_meta_requsted();
    }

    void register_managed_object_child(byte* child) const
    {
        m_new_stack.register_child(child);
    }

    gc_ptr_offsets_range gc_ptr_offsets() const
    {
        return m_new_stack.offsets();
    }

    void push_on_gc_new_stack(gc_new_stack::stack_entry* entry)
    {
        m_new_stack.push_entry(entry);
    }

    void pop_from_gc_new_stack()
    {
        m_new_stack.pop_entry();
    }

    void register_root(ptrs::gc_untyped_ptr* root)
    {
        logging::debug() << "register root " << root;
        m_root_set.insert(root);
    }

    void deregister_root(ptrs::gc_untyped_ptr* root)
    {
        logging::debug() << "deregister root " << root;
        m_root_set.remove(root);
    }

    bool is_root(const ptrs::gc_untyped_ptr* ptr) const
    {
        // dirty hack with const_cast here
        // should be refactored
        return m_root_set.contains(const_cast<ptrs::gc_untyped_ptr*>(ptr));
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
        if (!m_pin_stack.is_full()) {
            m_pin_stack.push_pin(ptr);
        } else {
            m_pin_set.insert(ptr);
        }
    }

    void pop_pin(byte* ptr)
    {
        if (m_pin_stack.top() == ptr) {
            m_pin_stack.pop_pin();
        } else {
            m_pin_set.remove(ptr);
        }
    }

    size_t pins_count() const
    {
        return m_pin_set.count() + m_pin_stack.count() + m_new_stack.depth();
    }

    template <typename Functor>
    void trace_pins(Functor&& f) const
    {
        m_pin_set.trace(std::forward<Functor>(f));
        m_pin_stack.trace(std::forward<Functor>(f));
        m_new_stack.trace_pointers(std::forward<Functor>(f));
    }

    std::thread::native_handle_type m_native_handle;
    std::thread::id m_id;
    root_stack_map m_root_set;
    pin_stack_map m_pin_set;
    pin_stack m_pin_stack;
    gc_new_stack m_new_stack;
    collectors::packet_manager::mark_packet_handle m_mark_packet;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_HPP
