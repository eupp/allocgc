#include <libprecisegc/details/serial_gc.hpp>

#include <utility>

#include <libprecisegc/details/gc_unsafe_scope.h>

namespace precisegc { namespace details {

serial_gc::serial_gc(gc_compacting compacting, std::unique_ptr<initation_policy>&& init_policy)
    : m_initator(this, std::move(init_policy))
{}

managed_ptr serial_gc::allocate(size_t size)
{
//    return m_heap.allocate(size).first;
    return managed_ptr();
}

byte* serial_gc::rbarrier(const atomic_byte_ptr& p)
{
    return p.load(std::memory_order_relaxed);
}

void serial_gc::wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    gc_unsafe_scope unsafe_scope;
    dst.store(src.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

void serial_gc::initation_point(initation_point_type ipoint)
{
    m_initator.initation_point(ipoint);
}

gc_stat serial_gc::stat() const
{
    gc_stat stat;
//    stat.heap_size                  = m_heap.size();
    stat.heap_size                  = 0;
    stat.incremental                = false;
    stat.support_concurrent_mark    = false;
    stat.support_concurrent_sweep   = false;
    return stat;
}

void serial_gc::gc()
{

}

}}

