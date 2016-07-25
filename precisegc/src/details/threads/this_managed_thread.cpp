#include <libprecisegc/details/threads/this_managed_thread.hpp>

#include <libprecisegc/details/threads/managed_thread_accessor.hpp>

namespace precisegc { namespace details { namespace threads {

managed_thread* this_managed_thread::this_thread = nullptr;

void this_managed_thread::register_root(ptrs::gc_untyped_ptr* root)
{
    managed_thread_accessor::root_set(this_thread).insert(root);
}

void this_managed_thread::deregister_root(ptrs::gc_untyped_ptr* root)
{
    managed_thread_accessor::root_set(this_thread).remove(root);
}

void this_managed_thread::pin(byte* ptr)
{
    managed_thread_accessor::pin_set(this_thread).insert(ptr);
}

void this_managed_thread::unpin(byte* ptr)
{
    managed_thread_accessor::pin_set(this_thread).remove(ptr);
}

}}}