#ifndef DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
#define DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>

namespace precisegc { namespace details {

class gc_alloc_descriptor
{
public:
    gc_alloc_descriptor() = delete;
    gc_alloc_descriptor(const gc_alloc_descriptor&) = default;
    gc_alloc_descriptor& operator=(const gc_alloc_descriptor&) = default;

    gc_alloc_descriptor(byte* ptr, size_t size, memory_descriptor* descr)
        : m_ptr(ptr)
        , m_size(size)
        , m_descr(descr)
    {}

    inline byte* get() const noexcept
    {
        return m_ptr;
    }

    inline size_t size() const noexcept
    {
        return m_size;
    }

    inline memory_descriptor* descriptor() const noexcept
    {
        return m_descr;
    }
private:
    byte*  m_ptr;
    size_t m_size;
    memory_descriptor* m_descr;
};

}}

#endif //DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
