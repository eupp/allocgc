#include <libprecisegc/details/allocators/gc_object_descriptor.hpp>

#include <cassert>

#include <libprecisegc/details/allocators/gc_box.hpp>

namespace precisegc { namespace details { namespace allocators {

gc_object_descriptor::gc_object_descriptor(size_t size)
    : m_size(size)
    , m_mark_bit(false)
    , m_pin_bit(false)
{ }

gc_object_descriptor::~gc_object_descriptor()
{}

bool gc_object_descriptor::get_mark() const noexcept
{
    return m_mark_bit;
}

bool gc_object_descriptor::get_pin() const noexcept
{
    return m_pin_bit;
}

void gc_object_descriptor::set_mark(bool mark) noexcept
{
    m_mark_bit = mark;
}

void gc_object_descriptor::set_pin(bool pin) noexcept
{
    m_pin_bit = pin;
}

bool gc_object_descriptor::get_mark(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_mark_bit;
}

bool gc_object_descriptor::get_pin(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_pin_bit;
}

void gc_object_descriptor::set_mark(byte* ptr, bool mark)
{
    assert(check_ptr(ptr));
    m_mark_bit = mark;
}

void gc_object_descriptor::set_pin(byte* ptr, bool pin)
{
    assert(check_ptr(ptr));
    m_pin_bit = pin;
}

gc_gen gc_object_descriptor::get_gen(byte* ptr) const
{
    assert(check_ptr(ptr));
    return GC_OLD_GEN;
}

gc_lifetime_tag gc_object_descriptor::get_lifetime_tag(byte* ptr) const
{
    return m_mark_bit ? gc_lifetime_tag::LIVE : gc_lifetime_tag::FREE;
}

size_t gc_object_descriptor::cell_size() const
{
    return m_size;
}

size_t gc_object_descriptor::cell_size(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_size;
}

byte* gc_object_descriptor::cell_start(byte* ptr) const
{
//    assert(check_ptr(ptr));
    ptrdiff_t offset = ptr - const_cast<byte*>(reinterpret_cast<const byte*>(this));
    return ptr - offset + sizeof(gc_object_descriptor);
}

size_t gc_object_descriptor::object_count(byte* ptr) const
{
    assert(check_ptr(ptr));
    return gc_box::get_obj_count(ptr);
}

const gc_type_meta* gc_object_descriptor::get_type_meta(byte* ptr) const
{
    assert(check_ptr(ptr));
    return gc_box::get_type_meta(ptr);
}

byte* gc_object_descriptor::init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
{
    assert(check_ptr(ptr));
    return gc_box::create(ptr, obj_count, type_meta);
}

void gc_object_descriptor::commit(byte* ptr)
{
    assert(check_ptr(ptr));
    assert(gc_box::get_type_meta(ptr));
}

void gc_object_descriptor::commit(byte* ptr, const gc_type_meta* type_meta)
{
    assert(type_meta);
    assert(check_ptr(ptr));
    gc_box::set_type_meta(ptr, type_meta);
}

void gc_object_descriptor::trace(byte* ptr, const gc_trace_callback& cb) const
{
    assert(check_ptr(ptr));
    assert(get_lifetime_tag(ptr) == gc_lifetime_tag::LIVE);
    gc_box::trace(ptr, cb);
}

void gc_object_descriptor::move(byte* to, byte* from, gc_memory_descriptor* from_descr)
{
    assert(check_ptr(to));
    assert(get_lifetime_tag(to) == gc_lifetime_tag::FREE);
    assert(from_descr->get_lifetime_tag(from) == gc_lifetime_tag::LIVE);
    gc_box::move(to, from, from_descr->object_count(from), from_descr->get_type_meta(from));
    from_descr->set_mark(from, false);
    m_mark_bit = true;
}

void gc_object_descriptor::finalize(byte* ptr)
{
    assert(check_ptr(ptr));
//    assert(get_lifetime_tag(ptr) == gc_lifetime_tag::GARBAGE);
    gc_box::destroy(ptr);
    m_mark_bit = false;
}

gc_memory_descriptor* gc_object_descriptor::descriptor()
{
    return this;
}

bool gc_object_descriptor::check_ptr(byte* ptr) const
{
    return ptr == (reinterpret_cast<const byte*>(this) + sizeof(gc_object_descriptor));
}

}}}
