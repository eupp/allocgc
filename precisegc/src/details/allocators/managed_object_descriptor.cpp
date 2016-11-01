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
    using namespace collectors;
    m_init_bit = true;
    traceable_object_meta* meta = managed_object::get_meta(ptr);
    new (meta) traceable_object_meta(rqst.obj_count(), rqst.type_meta());
    return gc_alloc_response(managed_object::get_object(ptr), rqst.alloc_size(), this);
}

size_t managed_object_descriptor::destroy(byte* ptr)
{
    using namespace collectors;

    assert(check_ptr(ptr));
    if (m_init_bit) {
        traceable_object_meta* meta = managed_object::get_meta(ptr);
        meta->get_type_meta()->destroy(ptr);
        m_init_bit = false;
        return m_size;
    }
    return 0;
}

bool managed_object_descriptor::get_mark() const noexcept
{
    return m_mark_bit;
}

bool managed_object_descriptor::get_pin() const noexcept
{
    return m_pin_bit;
}

bool managed_object_descriptor::set_mark(bool mark) noexcept
{
    m_mark_bit = mark;
}

bool managed_object_descriptor::set_pin(bool pin) noexcept
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

size_t managed_object_descriptor::object_count(byte* ptr) const
{
    return get_meta(ptr)->object_count();
}

void managed_object_descriptor::set_object_count(byte* ptr, size_t cnt) const
{
    get_meta(ptr)->set_object_count(cnt);
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

collectors::traceable_object_meta* managed_object_descriptor::get_meta(byte* cell_start) const
{
    assert(check_ptr(cell_start));
    return collectors::managed_object(cell_start).meta();
}

}}}
