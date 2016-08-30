#ifndef DIPLOMA_INCREMENTAL_GC_HPP
#define DIPLOMA_INCREMENTAL_GC_HPP

#include <atomic>

#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/collectors/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_heap.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

class incremental_gc_base : public gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_gc_base(gc_compacting compacting, size_t threads_available);

    managed_ptr allocate(size_t size) override;

    byte* rbarrier(const gc_handle& handle) override;
    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    void interior_wbarrier(gc_handle& handle, byte* ptr) override;
    void interior_shift(gc_handle& handle, ptrdiff_t shift) override;

    gc_stats gc(const gc_options& options) override;
private:
    void flush_threads_packets(const threads::world_snapshot& snapshot);
    gc_stats start_marking();
    gc_stats sweep();

    gc_heap m_heap;
    packet_manager m_packet_manager;
    marker m_marker;
    size_t m_threads_available;
    gc_phase m_phase;
};

}

class incremental_gc : public internals::incremental_gc_base
{
public:
    explicit incremental_gc(size_t threads_available);

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

class incremental_compacting_gc : public internals::incremental_gc_base
{
public:
    explicit incremental_compacting_gc(size_t threads_available);

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

}}}

#endif //DIPLOMA_INCREMENTAL_GC_HPP
