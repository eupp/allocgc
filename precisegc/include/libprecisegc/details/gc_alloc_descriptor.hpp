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

    gc_alloc_descriptor(std::nullptr_t)
        : m_ptr(nullptr)
        , m_size(0)
        , m_descr(nullptr)
    {}

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

    bool get_mark() const
    {
        assert(m_descr);
        return m_descr->get_mark(m_ptr);
    }

    bool get_pin() const
    {
        assert(m_descr);
        return m_descr->get_pin(m_ptr);
    }

    void set_mark(bool mark) const
    {
        assert(m_descr);
        return m_descr->set_mark(m_ptr, mark);
    }

    void set_pin(bool pin) const
    {
        assert(m_descr);
        return m_descr->set_pin(m_ptr, pin);
    }
private:
    byte*  m_ptr;
    size_t m_size;
    memory_descriptor* m_descr;
};

}}

#endif //DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
