#include <libprecisegc/details/threads/managed_thread_accessor.hpp>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>

namespace precisegc { namespace details { namespace threads {

root_stack_map& managed_thread_accessor::root_set(managed_thread* thread)
{
    return thread->m_root_set;
}

pin_stack_map& managed_thread_accessor::pin_set(managed_thread* thread)
{
    return thread->m_pin_set;
}

void managed_thread_accessor::set_this_managed_thread_pointer(managed_thread* thread)
{
    this_managed_thread::this_thread = thread;
}

}}}

