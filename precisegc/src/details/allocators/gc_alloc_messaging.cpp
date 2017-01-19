#include <libprecisegc/details/allocators/gc_alloc_messaging.hpp>

#include <libprecisegc/details/gc_hooks.hpp>

namespace precisegc { namespace details { namespace allocators {

gc_alloc_request::gc_alloc_request()
    : m_obj_size(0)
    , m_obj_cnt(0)
    , m_type_meta(nullptr)
{}

gc_alloc_request::gc_alloc_request(std::nullptr_t)
    : gc_alloc_request()
{}

gc_alloc_request::gc_alloc_request(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
    : m_obj_size(obj_size)
    , m_obj_cnt(obj_cnt)
    , m_alloc_size(obj_size * obj_cnt)
    , m_type_meta(tmeta)
{}

gc_alloc_response::gc_alloc_response()
    : m_obj_start(nullptr)
    , m_size(0)
{}

gc_alloc_response::gc_alloc_response(std::nullptr_t)
    : gc_alloc_response()
{}

gc_alloc_response::gc_alloc_response(byte* obj_start, size_t size, const gc_cell& cell)
    : m_obj_start(obj_start)
    , m_size(size)
    , m_cell(cell)
{}

bool gc_alloc_response::get_mark() const
{
    return m_cell.get_mark();
}

void gc_alloc_response::set_mark(bool mark)
{
    m_cell.set_mark(mark);
}

bool gc_alloc_response::get_pin() const
{
    return m_cell.get_pin();
}

void gc_alloc_response::set_pin(bool pin)
{
    m_cell.set_pin(pin);
}

gc_lifetime_tag gc_alloc_response::get_lifetime_tag() const
{
    m_cell.get_lifetime_tag();
}

void gc_alloc_response::commit()
{
    gc_commit(m_cell);
}

void gc_alloc_response::commit(const gc_type_meta* type_meta)
{
    gc_commit(m_cell, type_meta);
}

}}}

