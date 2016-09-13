#include <libprecisegc/details/threads/this_managed_thread.hpp>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/managed_thread_accessor.hpp>

namespace precisegc { namespace details { namespace threads {

thread_local managed_thread* this_managed_thread::this_thread = nullptr;

managed_thread* this_managed_thread::thread_ptr()
{
    return this_thread;
}

std::thread::id this_managed_thread::get_id()
{
    return this_thread->get_id();
}

std::thread::native_handle_type this_managed_thread::get_native_handle()
{
    return this_thread->native_handle();
}

void this_managed_thread::register_root(ptrs::gc_untyped_ptr* root)
{
    managed_thread_accessor::get_root_set(this_thread).insert(root);
}

void this_managed_thread::deregister_root(ptrs::gc_untyped_ptr* root)
{
    managed_thread_accessor::get_root_set(this_thread).remove(root);
}

void this_managed_thread::pin(byte* ptr)
{
    managed_thread_accessor::get_pin_set(this_thread).insert(ptr);
}

void this_managed_thread::unpin(byte* ptr)
{
    managed_thread_accessor::get_pin_set(this_thread).remove(ptr);
}

void this_managed_thread::push_pin(byte* ptr)
{
    managed_thread_accessor::get_pin_stack(this_thread).push_pin(ptr);
}

void this_managed_thread::pop_pin()
{
    managed_thread_accessor::get_pin_stack(this_thread).pop_pin();
}

collectors::packet_manager::mark_packet_handle& this_managed_thread::get_mark_packet()
{
    return this_thread->get_mark_packet();
}

}}}