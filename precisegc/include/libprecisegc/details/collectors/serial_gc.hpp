#ifndef DIPLOMA_SERIAL_GC_HPP
#define DIPLOMA_SERIAL_GC_HPP

#include <memory>

#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/collectors/marker.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_heap.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

class serial_gc_base : public gc_strategy, private utils::noncopyable, private utils::nonmovable
{
public:
    serial_gc_base(gc_compacting compacting, size_t threads_available);

    managed_ptr allocate(size_t size) override;

    byte* rbarrier(const gc_handle& handle) override;

    void interior_wbarrier(gc_handle& handle, byte* ptr) override;
    void interior_shift(gc_handle& handle, ptrdiff_t shift) override;

    gc_stats gc(const gc_options& options) override;
private:
    gc_heap m_heap;
    packet_manager m_packet_manager;
    marker m_marker;
    size_t m_threads_available;
};

}

class serial_gc : public internals::serial_gc_base
{
public:
    explicit serial_gc(size_t threads_available);

    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

class serial_compacting_gc : public internals::serial_gc_base
{
public:
    explicit serial_compacting_gc(size_t threads_available);

    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    bool compare(const gc_handle& a, const gc_handle& b) override;

    byte* pin(const gc_handle& handle) override;
    void  unpin(byte* ptr) override;

    gc_info info() const override;
};

}}}

#endif //DIPLOMA_SERIAL_GC_HPP
