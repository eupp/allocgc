#ifndef DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
#define DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>

namespace precisegc { namespace details {

class gc_alloc_request
{
public:
    gc_alloc_request()
        : m_obj_size(0)
        , m_obj_cnt(0)
        , m_tmeta(nullptr)
    {}

    gc_alloc_request(std::nullptr_t)
        : gc_alloc_request()
    {}

    gc_alloc_request(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
        : m_obj_size(obj_size)
        , m_obj_cnt(obj_cnt)
        , m_alloc_size(obj_size * obj_cnt)
        , m_tmeta(tmeta)
    {}

    gc_alloc_request(const gc_alloc_request&) noexcept = default;

    gc_alloc_request& operator=(const gc_alloc_request&) noexcept = default;

    size_t alloc_size() const noexcept
    {
        return m_alloc_size;
    }

    size_t obj_size() const noexcept
    {
        return m_obj_size;
    }

    size_t obj_count() const noexcept
    {
        return m_obj_cnt;
    }

    const gc_type_meta* type_meta() const noexcept
    {
        return m_tmeta;
    }
private:
    size_t m_obj_size;
    size_t m_obj_cnt;
    size_t m_alloc_size;
    const gc_type_meta* m_tmeta;
};


class gc_alloc_response
{
public:
    gc_alloc_response() = delete;
    gc_alloc_response(const gc_alloc_response&) = default;
    gc_alloc_response& operator=(const gc_alloc_response&) = default;

    gc_alloc_response(std::nullptr_t)
        : m_ptr(nullptr)
        , m_size(0)
        , m_descr(nullptr)
    {}

    gc_alloc_response(byte* ptr, size_t size, memory_descriptor* descr)
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
