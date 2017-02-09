#ifndef DIPLOMA_GC_CORE_HPP
#define DIPLOMA_GC_CORE_HPP

#include <libprecisegc/gc_new_stack_entry.hpp>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_heap.hpp>
#include <libprecisegc/details/gc_strategy.hpp>

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

class gc_core : public gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_core(const thread_descriptor& main_thrd_descr);

    allocators::gc_alloc_response allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta) override;

    byte* rbarrier(const gc_handle& handle) override;

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset) override;

    void register_handle(gc_handle& handle, byte* ptr) override;
    void deregister_handle(gc_handle& handle) override;

    byte* register_pin(const gc_handle& handle) override;
    void  deregister_pin(byte* pin) override;

    byte* push_pin(const gc_handle& handle) override;
    void  pop_pin(byte* pin) override;

    void register_stack_entry(gc_new_stack_entry* stack_entry) override;
    void deregister_stack_entry(gc_new_stack_entry* stack_entry) override;

    void register_thread(const thread_descriptor& descr) override;
    void deregister_thread(std::thread::id id) override;
protected:
    static_root_set* get_static_roots();

    void destroy_unmarked_dptrs();

    threads::world_snapshot stop_the_world();

    gc_heap_stat collect(const threads::world_snapshot& snapshot, size_t threads_available);
private:
    static thread_local threads::gc_thread_descriptor* this_thread;

    threads::gc_thread_manager m_thread_manager;
    static_root_set m_static_roots;
    dptr_storage m_dptr_storage;
    gc_heap m_heap;
};

}}}

#endif //DIPLOMA_GC_CORE_HPP
