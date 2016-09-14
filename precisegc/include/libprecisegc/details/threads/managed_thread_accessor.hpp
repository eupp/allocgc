#ifndef DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP
#define DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class thread_manager;
class world_snapshot;
class managed_thread;
class this_managed_thread;

class managed_thread_accessor : private utils::noncopyable, private utils::nonmovable
{
public:
    managed_thread_accessor() = delete;
private:
    static void register_root(managed_thread* thread, ptrs::gc_untyped_ptr* root);
    static void deregister_root(managed_thread* thread, ptrs::gc_untyped_ptr* root);

    static size_t roots_count(const managed_thread* thread);

    template <typename Functor>
    static void trace_roots(const managed_thread* thread, Functor&& f)
    {
        thread->trace_roots(std::forward<Functor>(f));
    }

    static void pin(managed_thread* thread, byte* ptr);
    static void unpin(managed_thread* thread, byte* ptr);

    static void push_pin(managed_thread* thread, byte* ptr);
    static void pop_pin(managed_thread* thread, byte* ptr);

    static size_t pins_count(const managed_thread* thread);

    template <typename Functor>
    static void trace_pins(const managed_thread* thread, Functor&& f)
    {
        thread->trace_pins(std::forward<Functor>(f));
    }

    static void set_this_managed_thread_pointer(managed_thread* thread);

    friend class thread_manager;
    friend class world_snapshot;
    friend class managed_thread;
    friend class this_managed_thread;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP
