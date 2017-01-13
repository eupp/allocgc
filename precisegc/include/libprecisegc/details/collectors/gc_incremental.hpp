#ifndef DIPLOMA_INCREMENTAL_GC_HPP
#define DIPLOMA_INCREMENTAL_GC_HPP

#include <atomic>

#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/collectors/dptr_storage.hpp>
#include <libprecisegc/details/collectors/remset.hpp>
#include <libprecisegc/details/collectors/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_heap.hpp>
#include <libprecisegc/details/allocators/gc_alloc_messaging.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_incremental : public gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_incremental(size_t threads_available);

    allocators::gc_alloc_response allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta) override;

    void commit(gc_cell& cell) override;
    void commit(gc_cell& cell, const gc_type_meta* type_meta) override;

    byte* init_ptr(byte* ptr, bool root_flag) override;

    byte* rbarrier(const gc_handle& handle) override;
    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset) override;

    gc_run_stats gc(const gc_options& options) override;

    gc_info info() const override;
private:
    gc_run_stats start_marking();
    gc_run_stats sweep();

    gc_heap m_heap;
    dptr_storage m_dptr_storage;
    packet_manager m_packet_manager;
    remset m_remset;
    marker m_marker;
    size_t m_threads_available;
    gc_phase m_phase;
};

}}}

#endif //DIPLOMA_INCREMENTAL_GC_HPP
