#ifndef DIPLOMA_GC_UNTYPED_PIN_H
#define DIPLOMA_GC_UNTYPED_PIN_H

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>

namespace precisegc { namespace details { namespace ptrs {

class gc_untyped_ptr;

class gc_untyped_pin
{
public:
    gc_untyped_pin(const gc_untyped_ptr& ptr);
    ~gc_untyped_pin();

    void* get() const noexcept;
protected:
    void* m_ptr;
};

}}}

#endif //DIPLOMA_GC_UNTYPED_PIN_H
