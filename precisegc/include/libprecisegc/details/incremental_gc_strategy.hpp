#ifndef DIPLOMA_INCREMENTAL_GC_HPP
#define DIPLOMA_INCREMENTAL_GC_HPP

#include <atomic>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_heap.h>
#include <libprecisegc/details/initator.hpp>
#include <libprecisegc/details/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class incremental_gc_strategy : public incremental_gc_interface
                              , private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_gc_strategy(gc_compacting compacting, std::unique_ptr<incremental_initation_policy> init_policy);

    managed_ptr allocate(size_t size) override;

    byte* rbarrier(const atomic_byte_ptr& p) override;
    void  wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src) override;

    void initation_point(initation_point_type ipoint) override;

    gc_info info() const override;

    gc_phase phase() const override;

    void gc() override;
    void incremental_gc(const incremental_gc_ops& ops) override;
private:
    void set_phase(gc_phase phase);

    void start_marking();
    void sweep();

    incremental_initator m_initator;
    gc_heap m_heap;
    marker m_marker;
    std::atomic<gc_phase> m_phase;
};

}}

#endif //DIPLOMA_INCREMENTAL_GC_HPP
