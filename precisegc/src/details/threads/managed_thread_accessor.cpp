#include <libprecisegc/details/threads/managed_thread_accessor.hpp>

#include <libprecisegc/details/threads/this_managed_thread.hpp>

namespace precisegc { namespace details { namespace threads {

bool managed_thread_accessor::is_stack_ptr(const managed_thread* thread, const gc_handle* ptr)
{
    return thread->is_stack_ptr(ptr);
}

bool managed_thread_accessor::is_heap_ptr(const managed_thread* thread, const gc_handle* ptr)
{
    return thread->is_heap_ptr(ptr);
}

bool managed_thread_accessor::is_type_meta_requested(const managed_thread* thread)
{
    return thread && thread->is_type_meta_requested();
}

void managed_thread_accessor::register_managed_object_child(managed_thread* thread, gc_handle* child)
{
    thread->register_managed_object_child(child);
}

managed_thread::gc_ptr_offsets_range managed_thread_accessor::gc_ptr_offsets(managed_thread* thread)
{
    return thread->gc_ptr_offsets();
}

void managed_thread_accessor::push_on_gc_new_stack(managed_thread* thread, gc_new_stack::stack_entry* entry)
{
    thread->push_on_gc_new_stack(entry);
}

void managed_thread_accessor::pop_from_gc_new_stack(managed_thread* thread)
{
    thread->pop_from_gc_new_stack();
}

void managed_thread_accessor::register_root(managed_thread* thread, gc_handle* root)
{
    thread->register_root(root);
}

void managed_thread_accessor::deregister_root(managed_thread* thread, gc_handle* root)
{
    thread->deregister_root(root);
}

bool managed_thread_accessor::is_root(const managed_thread* thread, const gc_handle* ptr)
{
    return thread->is_root(ptr);
}

size_t managed_thread_accessor::roots_count(const managed_thread* thread)
{
    return thread->roots_count();
}

void managed_thread_accessor::pin(managed_thread* thread, byte* ptr)
{
    thread->pin(ptr);
}

void managed_thread_accessor::unpin(managed_thread* thread, byte* ptr)
{
    thread->unpin(ptr);
}

void managed_thread_accessor::push_pin(managed_thread* thread, byte* ptr)
{
    thread->push_pin(ptr);
}

void managed_thread_accessor::pop_pin(managed_thread* thread, byte* ptr)
{
    thread->pop_pin(ptr);
}

size_t managed_thread_accessor::pins_count(const managed_thread* thread)
{
    return thread->pins_count();
}

void managed_thread_accessor::set_this_managed_thread_pointer(managed_thread* thread)
{
    this_managed_thread::this_thread = thread;
}

}}}

