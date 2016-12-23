#include <libprecisegc/details/allocators/managed_object_descriptor.hpp>

#include <cassert>

#include <libprecisegc/details/allocators/gc_box.hpp>

namespace precisegc { namespace details { namespace allocators {

managed_object_descriptor::managed_object_descriptor(size_t size)
    : m_size(size)
    , m_mark_bit(false)
    , m_pin_bit(false)
    , m_init_bit(false)
{ }

managed_object_descriptor::~managed_object_descriptor()
{}

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

gc_lifetime_tag managed_object_descriptor::get_lifetime_tag(byte* ptr) const
{
    return get_lifetime_tag_by_bits(m_mark_bit, m_init_bit);
}

size_t managed_object_descriptor::cell_size() const
{
    return m_size;
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
    assert(check_ptr(ptr));
    return gc_box::get_obj_count(ptr);
}

const gc_type_meta* managed_object_descriptor::get_type_meta(byte* ptr) const
{
    assert(check_ptr(ptr));
    return gc_box::get_type_meta(ptr);
}

byte* managed_object_descriptor::init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
{
    assert(check_ptr(ptr));
    return gc_box::create(ptr, obj_count, type_meta);
}

void managed_object_descriptor::mark_initilized(byte* ptr, const gc_type_meta* type_meta = nullptr)
{
    assert(check_ptr(ptr));
    assert(type_meta || gc_box::get_type_meta(ptr));
    gc_box::set_type_meta(ptr, type_meta);
    m_init_bit = true;
}

void managed_object_descriptor::move(byte* to, byte* from, memory_descriptor* from_descr)
{
    assert(check_ptr(to));
    assert(get_lifetime_tag(to) == gc_lifetime_tag::FREE);
    assert(from_descr->get_type_meta(from) != nullptr);
    gc_box::create(to, from_descr->object_count(from), from_descr->get_type_meta(from));
}

void managed_object_descriptor::finalize(byte* ptr)
{
    assert(check_ptr(ptr));
    assert(m_init_bit);
    gc_box::destroy(ptr);
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
