#ifndef DIPLOMA_GC_CORE_HPP
#define DIPLOMA_GC_CORE_HPP

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/gc_thread_descriptor.hpp>
#include <libprecisegc/details/threads/static_root_set.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_core : private utils::noncopyable, private utils::nonmovable
{
public:

    void register_handle(gc_handle& handle, byte* ptr)
    {
        if (this_thread->is_stack_ptr(&handle)) {
            this_thread->register_root(&handle);
        } else {
            m_static_roots.register_root(&handle);
        }
    }

    void deregister_handle(gc_handle& handle);

private:

    static thread_local threads::gc_thread_descriptor* this_thread;

    threads::thread_manager m_thread_manager;
    threads::static_root_set m_static_roots;

};

}}}

#endif //DIPLOMA_GC_CORE_HPP
