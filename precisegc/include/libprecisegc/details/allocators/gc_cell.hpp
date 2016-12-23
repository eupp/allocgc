#ifndef DIPLOMA_GC_CELL_HPP
#define DIPLOMA_GC_CELL_HPP

#include <libprecisegc/details/allocators/gc_box.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>

namespace precisegc { namespace details { namespace allocators {

class gc_cell
{
public:
    static gc_cell from_obj_start(byte* obj_start, memory_descriptor* descr)
    {
        return gc_cell(gc_box::cell_start(obj_start), descr);
    }

    static gc_cell from_cell_start(byte* cell_start, memory_descriptor* descr)
    {
        return gc_cell(cell_start, descr);
    }

    gc_cell() = delete;

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

    memory_descriptor* descriptor() const
    {
        return m_descr;
    }

    bool get_mark() const
    {
        return m_descr->get_mark(m_cell);
    }

    bool get_pin() const
    {
        return m_descr->get_pin(m_cell);
    }

    void set_mark(bool mark)
    {
        m_descr->set_mark(m_cell, mark);
    }

    void set_pin(bool pin)
    {
        m_descr->set_pin(m_cell, pin);
    }

    gc_lifetime_tag get_lifetime_tag() const
    {
        return m_descr->get_lifetime_tag(m_cell);
    }

    size_t cell_size() const
    {
        return m_descr->cell_size(m_cell);
    }

    byte* cell_start() const
    {
        return m_cell;
    }

    size_t object_count() const
    {
        return m_descr->object_count(m_cell);
    }

    const gc_type_meta* get_type_meta() const
    {
        return m_descr->get_type_meta(m_cell);
    }

    void mark_initilized(const gc_type_meta* tmeta = nullptr)
    {
        m_descr->mark_initilized(m_cell, tmeta);
    }

    void move(const gc_cell& to)
    {
        to.m_descr->move(to.m_cell, m_cell, m_descr);
    }

    void finalize()
    {
        m_descr->finalize(m_cell);
    }
private:
    gc_cell(byte* cell, memory_descriptor* descr)
        : m_cell(cell)
        , m_descr(descr)
    { }

    byte*              m_cell;
    memory_descriptor* m_descr;
};

}}}

#endif //DIPLOMA_GC_CELL_HPP
