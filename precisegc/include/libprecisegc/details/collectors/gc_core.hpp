#ifndef DIPLOMA_GC_CORE_HPP
#define DIPLOMA_GC_CORE_HPP

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/gc_new_stack_entry.hpp>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/collectors/pin_set.hpp>
#include <libprecisegc/details/collectors/pin_stack.hpp>
#include <libprecisegc/details/collectors/stack_bitmap.hpp>
#include <libprecisegc/details/collectors/gc_new_stack.hpp>
#include <libprecisegc/details/collectors/static_root_set.hpp>
#include <libprecisegc/details/collectors/dptr_storage.hpp>
#include <libprecisegc/details/collectors/gc_tagging.hpp>

#include <libprecisegc/details/threads/gc_thread_manager.hpp>
#include <libprecisegc/details/threads/gc_thread_descriptor.hpp>
#include <libprecisegc/details/threads/gc_thread_descriptor.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_core : private utils::noncopyable, private utils::nonmovable
{
public:
    byte* rbarrier(const gc_handle& handle);

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset);

    void register_handle(gc_handle& handle, byte* ptr);
    void deregister_handle(gc_handle& handle);

    byte* register_pin(const gc_handle& handle);
    void  deregister_pin(byte* pin);

    byte* push_pin(const gc_handle& handle);
    void  pop_pin(byte* pin);

    void register_stack_entry(gc_new_stack_entry* stack_entry);
    void deregister_stack_entry(gc_new_stack_entry* stack_entry);

    void register_thread(std::thread::id id, byte* stack_start_addr);
    void deregister_thread(std::thread::id id);
private:
    static thread_local threads::gc_thread_descriptor* this_thread;

    threads::gc_thread_manager m_thread_manager;
    static_root_set m_static_roots;
    dptr_storage m_dptr_storage;
};

}}}

#endif //DIPLOMA_GC_CORE_HPP
