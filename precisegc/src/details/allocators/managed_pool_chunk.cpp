#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>

#include <libprecisegc/details/allocators/gc_box.hpp>

namespace precisegc { namespace details { namespace allocators {

managed_pool_chunk::managed_pool_chunk(byte* chunk, size_t size, size_t cell_size)
    : m_memory(chunk)
    , m_size(size)
    , m_cell_size(cell_size)
{}

managed_pool_chunk::~managed_pool_chunk()
{}

byte* managed_pool_chunk::memory() const
{
    return m_memory;
}

size_t managed_pool_chunk::size() const
{
    return m_size;
}

memory_descriptor* managed_pool_chunk::descriptor()
{
    return this;
}

byte * managed_pool_chunk::init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return gc_box::create(ptr, obj_count, type_meta);
}

bool managed_pool_chunk::contains(byte* ptr) const
{
    byte* mem_begin = memory();
    byte* mem_end   = memory() + size();
    return (mem_begin <= ptr) && (ptr < mem_end);
}

bool managed_pool_chunk::unused() const
{
    return m_mark_bits.none();
}

size_t managed_pool_chunk::count_lived() const
{
    return m_init_bits.count();
}

size_t managed_pool_chunk::count_pinned() const
{
    return m_pin_bits.count();
}

void managed_pool_chunk::unmark()
{
    m_mark_bits.reset_all();
    m_pin_bits.reset_all();
}

double managed_pool_chunk::residency() const
{
    return static_cast<double>(m_mark_bits.count()) / m_mark_bits.size();
}

managed_pool_chunk::memory_range_type managed_pool_chunk::memory_range()
{
    return boost::make_iterator_range(begin(), end());
}

managed_pool_chunk::iterator managed_pool_chunk::begin()
{
    return iterator(memory(), this, m_cell_size);
}

managed_pool_chunk::iterator managed_pool_chunk::end()
{
    return iterator(memory() + size(), this, m_cell_size);
}

bool managed_pool_chunk::is_init(size_t idx) const
{
    return m_init_bits.get(idx);
}

void managed_pool_chunk::set_init(size_t idx, bool init)
{
    m_init_bits.set(idx, init);
}

bool managed_pool_chunk::get_mark(size_t idx) const
{
    return m_mark_bits.get(idx);
}

bool managed_pool_chunk::get_mark(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t ind = calc_cell_ind(ptr);
    return m_mark_bits.get(ind);
}

bool managed_pool_chunk::get_pin(size_t idx) const
{
    return m_pin_bits.get(idx);
}

bool managed_pool_chunk::get_pin(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t ind = calc_cell_ind(ptr);
    return m_pin_bits.get(ind);
}

void managed_pool_chunk::set_mark(size_t idx, bool mark)
{
    m_mark_bits.set(idx, mark);
}

void managed_pool_chunk::set_mark(byte* ptr, bool mark)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t ind = calc_cell_ind(ptr);
    m_mark_bits.set(ind, mark);
}

void managed_pool_chunk::set_pin(size_t idx, bool pin)
{
    m_pin_bits.set(idx, pin);
}

void managed_pool_chunk::set_pin(byte* ptr, bool pin)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t ind = calc_cell_ind(ptr);
    m_pin_bits.set(ind, pin);
}

gc_lifetime_tag managed_pool_chunk::get_lifetime_tag(byte* ptr) const
{
    size_t idx = calc_cell_ind(ptr);
    return get_lifetime_tag_by_bits(get_mark(idx), is_init(idx));
}

size_t managed_pool_chunk::cell_size() const
{
    return m_cell_size;
}

size_t managed_pool_chunk::cell_size(byte* ptr) const
{
    assert(contains(ptr));
    return m_cell_size;
}

byte* managed_pool_chunk::cell_start(byte* ptr) const
{
    assert(contains(ptr));
    std::uintptr_t uintptr = reinterpret_cast<std::uintptr_t>(ptr);
    uintptr -= uintptr % cell_size(ptr);
    assert(uintptr % cell_size(ptr) == 0);
    return reinterpret_cast<byte*>(uintptr);
}

size_t managed_pool_chunk::object_count(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return gc_box::get_obj_count(ptr);
}

const gc_type_meta* managed_pool_chunk::get_type_meta(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return gc_box::get_type_meta(ptr);
}

void managed_pool_chunk::mark_initilized(byte* ptr)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    assert(gc_box::get_type_meta(ptr));
    set_init(calc_cell_ind(ptr), true);
}

void managed_pool_chunk::mark_initilized(byte* ptr, const gc_type_meta* tmeta)
{
    assert(tmeta);
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    gc_box::set_type_meta(ptr, tmeta);
    set_init(calc_cell_ind(ptr), true);
}

void managed_pool_chunk::move(byte* to, byte* from, memory_descriptor* from_descr)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    assert(is_init(calc_cell_ind(ptr)));
    assert(get_lifetime_tag(to) == gc_lifetime_tag::FREE);
    assert(get_lifetime_tag(from) == gc_lifetime_tag::INITIALIZED);
    gc_box::move(to, from, from_descr->object_count(from), from_descr->get_type_meta(from));
}

void managed_pool_chunk::finalize(byte* ptr)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    assert(is_init(calc_cell_ind(ptr)));
    gc_box::destroy(ptr);
}

size_t managed_pool_chunk::calc_cell_ind(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return (ptr - memory()) / m_cell_size;
}

}}}
