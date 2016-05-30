#ifndef DIPLOMA_SERIAL_GC_HPP
#define DIPLOMA_SERIAL_GC_HPP

#include <memory>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_heap.h>
#include <libprecisegc/details/initator.hpp>
#include <libprecisegc/details/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class serial_gc : public serial_gc_interface, private utils::noncopyable, private utils::nonmovable
{
public:
    serial_gc(gc_compacting compacting, std::unique_ptr<initation_policy>&& init_policy);

    managed_ptr allocate(size_t size);

    byte* rbarrier(const atomic_byte_ptr& p);
    void  wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src);

    void initation_point(initation_point_type ipoint);

    gc_stat stat() const;

    void gc();
private:
    initator m_initator;
    gc_heap m_heap;
    marker m_marker;
};

}}

#endif //DIPLOMA_SERIAL_GC_HPP
