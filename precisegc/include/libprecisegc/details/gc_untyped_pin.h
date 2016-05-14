#ifndef DIPLOMA_GC_UNTYPED_PIN_H
#define DIPLOMA_GC_UNTYPED_PIN_H

#include "gc_untyped_ptr.h"
#include "managed_memory_descriptor.h"

namespace precisegc { namespace details {

class gc_untyped_ptr;

class gc_untyped_pin
{
public:
    gc_untyped_pin(const gc_untyped_ptr& ptr);
    ~gc_untyped_pin();

    void* get() const noexcept;
protected:
    const gc_untyped_ptr* m_ptr;
};

}}

#endif //DIPLOMA_GC_UNTYPED_PIN_H
