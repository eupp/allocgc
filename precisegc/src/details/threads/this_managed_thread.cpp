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
    managed_thread_accessor::register_root(this_thread, root);
}

void this_managed_thread::deregister_root(ptrs::gc_untyped_ptr* root)
{
    managed_thread_accessor::deregister_root(this_thread, root);
}

void this_managed_thread::pin(byte* ptr)
{
    managed_thread_accessor::pin(this_thread, ptr);
}

void this_managed_thread::unpin(byte* ptr)
{
    managed_thread_accessor::unpin(this_thread, ptr);
}

void this_managed_thread::push_pin(byte* ptr)
{
    managed_thread_accessor::push_pin(this_thread, ptr);
}

void this_managed_thread::pop_pin(byte* ptr)
{
    managed_thread_accessor::pop_pin(this_thread, ptr);
}

collectors::packet_manager::mark_packet_handle& this_managed_thread::get_mark_packet()
{
    return this_thread->get_mark_packet();
}

}}}