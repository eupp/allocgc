#include <liballocgc/details/allocators/gc_object_descriptor.hpp>

#include <cassert>

#include <liballocgc/details/allocators/gc_box.hpp>

namespace allocgc { namespace details { namespace allocators {

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

bool gc_object_descriptor::set_mark(bool mark) noexcept
{
    m_mark_bit = mark;
}

bool gc_object_descriptor::set_pin(bool pin) noexcept
{
    m_pin_bit = pin;
}

bool gc_object_descriptor::get_mark(byte* ptr) const
{
    assert(ptr == cell_start());
    return m_mark_bit;
}

bool gc_object_descriptor::get_pin(byte* ptr) const
{
    assert(ptr == cell_start());
    return m_pin_bit;
}

void gc_object_descriptor::set_mark(byte* ptr, bool mark)
{
    assert(ptr == cell_start());
    m_mark_bit = mark;
}

void gc_object_descriptor::set_pin(byte* ptr, bool pin)
{
    assert(ptr == cell_start());
    m_pin_bit = pin;
}

bool gc_object_descriptor::is_init(byte* ptr) const
{
    assert(ptr == cell_start());
    return m_init_bit;
}

gc_lifetime_tag gc_object_descriptor::get_lifetime_tag(byte* ptr) const
{
    assert(ptr == cell_start());
    return get_lifetime_tag_by_bits(m_mark_bit, m_init_bit);
}

size_t gc_object_descriptor::cell_size() const
{
    return m_size;
}

size_t gc_object_descriptor::cell_size(byte* ptr) const
{
    assert(ptr == cell_start());
    return m_size;
}

byte* gc_object_descriptor::cell_start() const
{
    const byte* start = reinterpret_cast<const byte*>(this) + sizeof(gc_object_descriptor);
    return const_cast<byte*>(start);
}

byte* gc_object_descriptor::cell_start(byte* ptr) const
{
    assert(check_ptr(ptr));
    return cell_start();
}

size_t gc_object_descriptor::object_count(byte* ptr) const
{
    assert(ptr == cell_start());
    return gc_box::get_obj_count(ptr);
}

const gc_type_meta* gc_object_descriptor::get_type_meta(byte* ptr) const
{
    assert(ptr == cell_start());
    return gc_box::get_type_meta(ptr);
}

byte* gc_object_descriptor::init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
{
    assert(ptr == cell_start());
    return gc_box::create(ptr, obj_count, type_meta);
}

void gc_object_descriptor::commit(byte* ptr)
{
    assert(ptr == cell_start());
    assert(gc_box::get_type_meta(ptr));
    m_init_bit = true;
}

void gc_object_descriptor::commit(byte* ptr, const gc_type_meta* type_meta)
{
    assert(type_meta);
    assert(ptr == cell_start());
    gc_box::set_type_meta(ptr, type_meta);
    m_init_bit = true;
}

void gc_object_descriptor::trace(byte* ptr, const gc_trace_callback& cb) const
{
    assert(ptr == cell_start());
    assert(is_init(ptr));
    gc_box::trace(ptr, cb);
}

void gc_object_descriptor::move(byte* to, byte* from, gc_memory_descriptor* from_descr)
{
    assert(to == cell_start());
    assert(get_lifetime_tag(to) == gc_lifetime_tag::FREE);
    assert(from_descr->get_lifetime_tag(from) == gc_lifetime_tag::LIVE);
    gc_box::move(to, from, from_descr->object_count(from), from_descr->get_type_meta(from));
    from_descr->set_mark(from, false);
    m_mark_bit = true;
    m_init_bit = true;
}

void gc_object_descriptor::finalize(byte* ptr)
{
    assert(ptr == cell_start());
    assert(m_init_bit);
    gc_box::destroy(cell_start());
    m_init_bit = false;
}

gc_memory_descriptor* gc_object_descriptor::descriptor()
{
    return this;
}

bool gc_object_descriptor::check_ptr(byte* ptr) const
{
    return (cell_start() <= ptr) && (ptr < cell_start() + m_size);
}

}}}
