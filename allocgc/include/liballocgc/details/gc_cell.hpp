#ifndef ALLOCGC_GC_CELL_HPP
#define ALLOCGC_GC_CELL_HPP

#include <liballocgc/details/allocators/gc_box.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>

namespace allocgc { namespace details {

class gc_cell
{
public:
    static gc_cell from_obj_start(byte* obj_start, allocators::gc_memory_descriptor* descr)
    {
        return gc_cell(allocators::gc_box::get_cell_start(obj_start), descr);
    }

    static gc_cell from_cell_start(byte* cell_start, allocators::gc_memory_descriptor* descr)
    {
        return gc_cell(cell_start, descr);
    }

    static gc_cell from_internal_ptr(byte* ptr, allocators::gc_memory_descriptor* descr)
    {
        return gc_cell(descr->cell_start(ptr), descr);
    }

    gc_cell()
        : m_cell(nullptr)
        , m_descr(nullptr)
    {}

    gc_cell(const gc_cell&) = default;
    gc_cell& operator=(const gc_cell&) = default;

    byte* get() const
    {
        return m_cell;
    }

    void reset(byte* cell_start)
    {
        m_cell = cell_start;
    }

    allocators::gc_memory_descriptor* descriptor() const
    {
        return m_descr;
    }

    bool get_mark() const
    {
        assert(is_initialized());
        return m_descr->get_mark(m_cell);
    }

    bool get_pin() const
    {
        assert(is_initialized());
        return m_descr->get_pin(m_cell);
    }

    void set_mark(bool mark) const
    {
        assert(is_initialized());
        m_descr->set_mark(m_cell, mark);
    }

    void set_pin(bool pin) const
    {
        assert(is_initialized());
        m_descr->set_pin(m_cell, pin);
    }

    bool is_init() const
    {
        assert(is_initialized());
        return m_descr->is_init(m_cell);
    }

    gc_lifetime_tag get_lifetime_tag() const
    {
        assert(is_initialized());
        return m_descr->get_lifetime_tag(m_cell);
    }

    size_t cell_size() const
    {
        assert(is_initialized());
        return m_descr->cell_size(m_cell);
    }

    byte* cell_start() const
    {
        return m_cell;
    }

    size_t object_count() const
    {
        assert(is_initialized());
        return m_descr->object_count(m_cell);
    }

    const gc_type_meta* get_type_meta() const
    {
        assert(is_initialized());
        return m_descr->get_type_meta(m_cell);
    }

    void trace(const gc_trace_callback& cb) const
    {
        assert(is_initialized());
        m_descr->trace(m_cell, cb);
    }

    void move(const gc_cell& to) const
    {
        assert(is_initialized());
        to.m_descr->move(to.m_cell, m_cell, m_descr);
    }

    void finalize() const
    {
        assert(is_initialized());
        m_descr->finalize(m_cell);
    }
private:
    gc_cell(byte* cell, allocators::gc_memory_descriptor* descr)
        : m_cell(cell)
        , m_descr(descr)
    {
        assert(is_initialized());
    }

    bool is_initialized() const
    {
        return m_cell && m_descr;
    }

    byte*                             m_cell;
    allocators::gc_memory_descriptor* m_descr;
};

}}

#endif //ALLOCGC_GC_CELL_HPP
