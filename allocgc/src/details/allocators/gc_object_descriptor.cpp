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

gc_memory_descriptor::box_id gc_object_descriptor::get_id(byte* ptr) const
{
    assert(check_ptr(ptr));
    return 0;
}

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

bool gc_object_descriptor::get_mark(box_id id) const
{
    assert(id == 0);
    return m_mark_bit;
}

bool gc_object_descriptor::get_pin(box_id id) const
{
    assert(id == 0);
    return m_pin_bit;
}

void gc_object_descriptor::set_mark(box_id id, bool mark)
{
    assert(id == 0);
    m_mark_bit = mark;
}

void gc_object_descriptor::set_pin(box_id id, bool pin)
{
    assert(id == 0);
    m_pin_bit = pin;
}

bool gc_object_descriptor::is_init(box_id id) const
{
    assert(id == 0);
    return m_init_bit;
}

gc_lifetime_tag gc_object_descriptor::get_lifetime_tag(box_id id) const
{
    assert(id == 0);
    return get_lifetime_tag_by_bits(m_mark_bit, m_init_bit);
}

size_t gc_object_descriptor::box_size() const
{
    return m_size;
}

size_t gc_object_descriptor::box_size(box_id id) const
{
    assert(id == 0);
    return m_size;
}

byte* gc_object_descriptor::box_addr() const
{
    const byte* start = reinterpret_cast<const byte*>(this) + sizeof(gc_object_descriptor);
    return const_cast<byte*>(start);
}

byte* gc_object_descriptor::box_addr(box_id id) const
{
    assert(id == 0);
    return box_addr();
}

size_t gc_object_descriptor::object_count(box_id id) const
{
    assert(id == 0);
    return gc_box::get_obj_count(box_addr());
}

const gc_type_meta* gc_object_descriptor::get_type_meta(box_id id) const
{
    assert(id == 0);
    return gc_box::get_type_meta(box_addr());
}

byte* gc_object_descriptor::init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
{
    assert(ptr == box_addr());
    return gc_box::create(ptr, obj_count, type_meta);
}

void gc_object_descriptor::commit(box_id id)
{
    assert(id == 0);
    assert(gc_box::get_type_meta(box_addr()));
    m_init_bit = true;
}

void gc_object_descriptor::commit(box_id id, const gc_type_meta* type_meta)
{
    assert(id == 0);
    assert(type_meta);
    gc_box::set_type_meta(box_addr(), type_meta);
    m_init_bit = true;
}

void gc_object_descriptor::trace(box_id id, const gc_trace_callback& cb) const
{
    assert(id == 0);
    assert(is_init(id));
    gc_box::trace(box_addr(), cb);
}

//void gc_object_descriptor::move(byte* to, byte* from, gc_memory_descriptor* from_descr)
//{
//    assert(to == box_addr());
//    assert(get_lifetime_tag(to) == gc_lifetime_tag::FREE);
//    assert(from_descr->get_lifetime_tag(from) == gc_lifetime_tag::LIVE);
//    gc_box::move(to, from, from_descr->object_count(from), from_descr->get_type_meta(from));
//    from_descr->set_mark(from, false);
//    m_mark_bit = true;
//    m_init_bit = true;
//}

void gc_object_descriptor::finalize(box_id id)
{
    assert(id == 0);
    assert(m_init_bit);
    gc_box::destroy(box_addr());
    m_init_bit = false;
}

gc_memory_descriptor* gc_object_descriptor::descriptor()
{
    return this;
}

bool gc_object_descriptor::check_ptr(byte* ptr) const
{
    return (box_addr() <= ptr) && (ptr < box_addr() + m_size);
}

}}}
