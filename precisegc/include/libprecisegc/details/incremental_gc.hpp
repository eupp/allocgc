#ifndef DIPLOMA_INCREMENTAL_GC_HPP
#define DIPLOMA_INCREMENTAL_GC_HPP

#include <atomic>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_heap.h>
#include <libprecisegc/details/initator.hpp>
#include <libprecisegc/details/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

namespace internals {

class incremental_gc_base : public incremental_gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_gc_base(gc_compacting compacting, std::unique_ptr<incremental_initation_policy> init_policy);

    managed_ptr allocate(size_t size) override;

    byte* rbarrier(const gc_handle& handle) override;
    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    void interior_wbarrier(gc_handle& handle, byte* ptr) override;
    void interior_shift(gc_handle& handle, ptrdiff_t shift) override;

    void initation_point(initation_point_type ipoint) override;

    gc_phase phase() const override;

    void gc() override;
    void gc_increment(const incremental_gc_ops& ops) override;
private:
    void set_phase(gc_phase phase);

    void start_marking();
    void sweep();

    incremental_initator m_initator;
    gc_heap m_heap;
    marker m_marker;
    std::atomic<gc_phase> m_phase;
};

}

class incremental_gc : public internals::incremental_gc_base
                     , private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_gc(gc_compacting compacting, std::unique_ptr<incremental_initation_policy> init_policy);

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

class incremental_compacting_gc : public internals::incremental_gc_base
                                , private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_compacting_gc(gc_compacting compacting, std::unique_ptr<incremental_initation_policy> init_policy);

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

}}

#endif //DIPLOMA_INCREMENTAL_GC_HPP
