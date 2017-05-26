#ifndef ALLOCGC_GC_CORE_HPP
#define ALLOCGC_GC_CORE_HPP

#include <liballocgc/details/gc_hooks.hpp>
#include <liballocgc/details/collectors/gc_heap.hpp>

#include <liballocgc/details/utils/make_unique.hpp>
#include <liballocgc/details/utils/utility.hpp>

#include <liballocgc/details/collectors/static_root_set.hpp>
#include <liballocgc/details/collectors/marker.hpp>

#include <liballocgc/details/threads/gc_thread_manager.hpp>
#include <liballocgc/details/threads/gc_thread_descriptor.hpp>

namespace allocgc { namespace details { namespace collectors {

class gc_core : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_core(remset* rset);

    ~gc_core();

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    void abort(const gc_alloc::response& rsp);
    void commit(const gc_alloc::response& rsp);
    void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta);

    gc_offsets make_offsets(const gc_alloc::response& rsp);

    byte* rbarrier(const gc_handle& handle);

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset);

    void register_handle(gc_handle& handle, byte* ptr);
    void deregister_handle(gc_handle& handle);

    byte* register_pin(const gc_handle& handle);
    void  deregister_pin(byte* pin);

    byte* push_pin(const gc_handle& handle);
    void  pop_pin(byte* pin);

    void register_thread(const thread_descriptor& descr);
    void deregister_thread(std::thread::id id);

    void set_heap_limit(size_t limit);
protected:
    threads::world_snapshot stop_the_world();

    void trace_roots(const threads::world_snapshot& snapshot);
    void trace_pins(const threads::world_snapshot& snapshot);
    void trace_uninit(const threads::world_snapshot& snapshot);
    void trace_remset();

    void start_concurrent_marking(size_t threads_available);
    void start_marking();

    gc_heap_stat collect(const threads::world_snapshot& snapshot, size_t threads_available);
private:
    void root_trace_cb(gc_handle* root);
    void pin_trace_cb(byte* ptr);
    void conservative_root_trace_cb(gc_handle* root);
    void conservative_obj_trace_cb(byte* obj_start, size_t obj_size);

    static thread_local threads::gc_thread_descriptor* this_thread;

    threads::gc_thread_manager m_thread_manager;
    static_root_set m_static_roots;
    packet_manager m_packet_manager;
    marker m_marker;
    gc_heap m_heap;
    bool m_conservative_mode;
};

}}}

#endif //ALLOCGC_GC_CORE_HPP
