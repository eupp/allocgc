#ifndef DIPLOMA_SERIAL_GC_HPP
#define DIPLOMA_SERIAL_GC_HPP

#include <memory>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_heap.h>
#include <libprecisegc/details/initator.hpp>
#include <libprecisegc/details/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

namespace internals {

class serial_gc_base : public serial_gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    serial_gc_base(gc_compacting compacting, std::unique_ptr<initation_policy> init_policy);

    managed_ptr allocate(size_t size) override;

    void initation_point(initation_point_type ipoint) override;

    void gc() override;
private:
    initator m_initator;
    gc_heap m_heap;
    marker m_marker;
};

}

class serial_gc : public internals::serial_gc_base
{
public:
    serial_gc(gc_compacting compacting, std::unique_ptr<initation_policy> init_policy);

    byte* rbarrier(const gc_handle& handle) override;
    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    void interior_wbarrier(gc_handle& handle, byte* ptr) override;
    void interior_shift(gc_handle& handle, ptrdiff_t shift) override;

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

class serial_compacting_gc : public internals::serial_gc_base
{
public:
    serial_compacting_gc(gc_compacting compacting, std::unique_ptr<initation_policy> init_policy);

    byte* rbarrier(const gc_handle& handle) override;
    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    void interior_wbarrier(gc_handle& handle, byte* ptr) override;
    void interior_shift(gc_handle& handle, ptrdiff_t shift) override;

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

}}

#endif //DIPLOMA_SERIAL_GC_HPP
