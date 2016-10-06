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
#include <libprecisegc/details/gc_alloc_descriptor.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

class incremental_gc_base : public gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_gc_base(gc_compacting compacting, size_t threads_available);

    gc_alloc_descriptor allocate(size_t obj_size, size_t obj_cnt, const type_meta* tmeta) override;
    void commit(const gc_alloc_descriptor& alloc_descr) override;

    byte* rbarrier(const gc_word& handle) override;
    void  wbarrier(gc_word& dst, const gc_word& src) override;

    void interior_wbarrier(gc_word& handle, ptrdiff_t offset) override;

    gc_run_stats gc(const gc_options& options) override;
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

}

class incremental_gc : public internals::incremental_gc_base
{
public:
    explicit incremental_gc(size_t threads_available);

    gc_info info() const override;
};

class incremental_compacting_gc : public internals::incremental_gc_base
{
public:
    explicit incremental_compacting_gc(size_t threads_available);

    void interior_wbarrier(gc_word& handle, ptrdiff_t offset) override;

    gc_info info() const override;
};

}}}

#endif //DIPLOMA_INCREMENTAL_GC_HPP
