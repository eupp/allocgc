#ifndef DIPLOMA_THIS_MANAGED_THREAD_HPP
#define DIPLOMA_THIS_MANAGED_THREAD_HPP

#include <thread>

#include <libprecisegc/details/threads/gc_new_stack.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/managed_ptr.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread;
class managed_thread_accessor;

class this_managed_thread : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef gc_new_stack::offsets_range gc_ptr_offsets_range;

    this_managed_thread() = delete;

    // for testing purpose
    static managed_thread* thread_ptr();

    static std::thread::id get_id();
    static std::thread::native_handle_type get_native_handle();

    static bool is_stack_ptr(const gc_handle* ptr);
    static bool is_heap_ptr(const gc_handle* ptr);
    static bool is_type_meta_requested();

    static void register_managed_object_child(gc_handle* child);
    static gc_ptr_offsets_range gc_ptr_offsets();

    static void push_on_gc_new_stack(gc_new_stack::stack_entry* top);
    static void pop_from_gc_new_stack();

    static void register_root(gc_handle* root);
    static void deregister_root(gc_handle* root);
    static bool is_root(const gc_handle* ptr);

    static void pin(byte* ptr);
    static void unpin(byte* ptr);

    static void push_pin(byte* ptr);
    static void pop_pin(byte* ptr);

    friend class managed_thread;
    friend class managed_thread_accessor;
private:
    static thread_local managed_thread* this_thread;
};

}}}

#endif //DIPLOMA_THIS_MANAGED_THREAD_HPP
