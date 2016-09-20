#ifndef DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP
#define DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/gc_new_stack.hpp>
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
    static bool is_heap_ptr(const managed_thread* thread, byte* ptr);
    static bool is_type_meta_requested(const managed_thread* thread);

    static void register_managed_object_child(managed_thread* thread, byte* child);
    static managed_thread::gc_ptr_offsets_range gc_ptr_offsets(managed_thread* thread);

    static void push_on_gc_new_stack(managed_thread* thread, gc_new_stack::stack_entry* entry);
    static void pop_from_gc_new_stack(managed_thread* thread);

    static void register_root(managed_thread* thread, gc_handle* root);
    static void deregister_root(managed_thread* thread, gc_handle* root);
    static bool is_root(const managed_thread* thread, const gc_handle* ptr);

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
