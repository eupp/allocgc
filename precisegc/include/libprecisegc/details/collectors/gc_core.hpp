#ifndef DIPLOMA_GC_CORE_HPP
#define DIPLOMA_GC_CORE_HPP

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_heap.hpp>
#include <libprecisegc/details/gc_strategy.hpp>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/collectors/static_root_set.hpp>
#include <libprecisegc/details/collectors/marker.hpp>

#include <libprecisegc/details/threads/gc_thread_manager.hpp>
#include <libprecisegc/details/threads/gc_thread_descriptor.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_core : public gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_core(const thread_descriptor& main_thrd_descr, remset* rset);

    ~gc_core();

    gc_alloc::response allocate(const gc_alloc::request& rqst) override;

    void abort(const gc_alloc::response& rsp) override;
    void commit(const gc_alloc::response& rsp) override;
    void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta) override;

    gc_offsets make_offsets(const gc_alloc::response& rsp) override;

    byte* rbarrier(const gc_handle& handle) override;

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset) override;

    void register_handle(gc_handle& handle, byte* ptr) override;
    void deregister_handle(gc_handle& handle) override;

    byte* register_pin(const gc_handle& handle) override;
    void  deregister_pin(byte* pin) override;

    byte* push_pin(const gc_handle& handle) override;
    void  pop_pin(byte* pin) override;

    void register_thread(const thread_descriptor& descr) override;
    void deregister_thread(std::thread::id id) override;
protected:
//    static_root_set* get_static_roots();

//    void destroy_unmarked_dptrs();

    threads::world_snapshot stop_the_world();

    void trace_roots(const threads::world_snapshot& snapshot);
    void trace_pins(const threads::world_snapshot& snapshot);
    void trace_remset();

    void start_concurrent_marking(size_t threads_available);
    void start_marking();

    gc_heap_stat collect(const threads::world_snapshot& snapshot, size_t threads_available);
private:
    void root_trace_cb(gc_handle* root);
    void conservative_root_trace_cb(gc_handle* root);
    void pin_trace_cb(byte* ptr);

    static thread_local threads::gc_thread_descriptor* this_thread;

    threads::gc_thread_manager m_thread_manager;
    static_root_set m_static_roots;
//    dptr_storage m_dptr_storage;
    packet_manager m_packet_manager;
    marker m_marker;
    gc_heap m_heap;
};

}}}

#endif //DIPLOMA_GC_CORE_HPP
