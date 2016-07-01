#ifndef DIPLOMA_INCREMENTAL_GC_HPP
#define DIPLOMA_INCREMENTAL_GC_HPP

#include <atomic>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_heap.h>
#include <libprecisegc/details/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

class incremental_gc_base : public gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_gc_base(gc_compacting compacting);

    managed_ptr allocate(size_t size) override;

    byte* rbarrier(const gc_handle& handle) override;
    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    void interior_wbarrier(gc_handle& handle, byte* ptr) override;
    void interior_shift(gc_handle& handle, ptrdiff_t shift) override;

    void gc(gc_phase) override;
private:
    void start_marking();
    void sweep();

    gc_heap m_heap;
    marker m_marker;
    gc_phase m_phase;
};

}

class incremental_gc : public internals::incremental_gc_base
{
public:
    incremental_gc();

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

class incremental_compacting_gc : public internals::incremental_gc_base
{
public:
    incremental_compacting_gc();

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

}}}

#endif //DIPLOMA_INCREMENTAL_GC_HPP
