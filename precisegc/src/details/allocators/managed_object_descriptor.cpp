#include <libprecisegc/details/allocators/managed_object_descriptor.hpp>

#include <cassert>

#include <libprecisegc/details/collectors/managed_object.hpp>

namespace precisegc { namespace details { namespace allocators {

managed_object_descriptor::managed_object_descriptor(size_t size)
    : m_size(size)
    , m_mark_bit(false)
    , m_pin_bit(false)
    , m_init_bit(false)
{
    assert(size > LARGE_CELL_SIZE);
}

managed_object_descriptor::~managed_object_descriptor()
{}

gc_alloc_response managed_object_descriptor::init(byte* ptr, const gc_alloc_request& rqst)
{
    object_meta* meta = reinterpret_cast<object_meta*>(ptr);
    meta->m_tmeta = rqst.type_meta();
    meta->m_obj_cnt = rqst.obj_count();
    return gc_alloc_response(ptr + sizeof(object_meta), rqst.alloc_size(), this);
}

size_t managed_object_descriptor::destroy(byte* ptr)
{
    assert(check_ptr(ptr));
    if (m_init_bit) {
        object_meta* meta = reinterpret_cast<object_meta*>(ptr);
        meta->m_tmeta->destroy(ptr);
        m_init_bit = false;
        return m_size;
    }
    return 0;
}

bool managed_object_descriptor::get_mark() const
{
    return m_mark_bit;
}

bool managed_object_descriptor::get_pin() const
{
    return m_pin_bit;
}

bool managed_object_descriptor::set_mark(bool mark)
{
    m_mark_bit = mark;
}

bool managed_object_descriptor::set_pin(bool pin)
{
    m_pin_bit = pin;
}

void managed_object_descriptor::set_initialized(byte* ptr)
{
    check_ptr(ptr);
    m_init_bit = true;
}

bool managed_object_descriptor::is_initialized(byte* ptr) const
{
    check_ptr(ptr);
    return m_init_bit;
}

bool managed_object_descriptor::get_mark(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_mark_bit;
}

bool managed_object_descriptor::get_pin(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_pin_bit;
}

void managed_object_descriptor::set_mark(byte* ptr, bool mark)
{
    assert(check_ptr(ptr));
    m_mark_bit = mark;
}

void managed_object_descriptor::set_pin(byte* ptr, bool pin)
{
    assert(check_ptr(ptr));
    m_pin_bit = pin;
}

size_t managed_object_descriptor::cell_size(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_size;
}

byte* managed_object_descriptor::cell_start(byte* ptr) const
{
    assert(check_ptr(ptr));
    return ptr;
}

const gc_type_meta* managed_object_descriptor::get_type_meta(byte* ptr) const
{
    assert(check_ptr(ptr));
    return collectors::managed_object(ptr).meta()->get_type_meta();
}

void managed_object_descriptor::set_type_meta(byte* ptr, const gc_type_meta* tmeta)
{
    assert(check_ptr(ptr));
    collectors::managed_object(ptr).meta()->set_type_meta(tmeta);
}

size_t managed_object_descriptor::size() const
{
    return m_size;
}

memory_descriptor* managed_object_descriptor::descriptor()
{
    return this;
}

bool managed_object_descriptor::check_ptr(byte* ptr) const
{
    return ptr == (reinterpret_cast<const byte*>(this) + sizeof(managed_object_descriptor));
}



}}}
