#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>

#include <cassert>
#include <limits>
#include <utility>

#include <libprecisegc/details/collectors/managed_object.hpp>
#include <libprecisegc/details/utils/math.hpp>
#include <libprecisegc/details/logging.hpp>

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

gc_alloc_response managed_pool_chunk::init(byte* ptr, const gc_alloc_request& rqst)
{
    using namespace collectors;

    assert(contains(ptr));
    assert(ptr == cell_start(ptr));

    traceable_object_meta* meta = managed_object::get_meta(ptr);
    new (meta) traceable_object_meta(rqst.obj_count(), rqst.type_meta());

    byte* obj = managed_object::get_object(ptr);
    set_init(obj, true);

    return gc_alloc_response(obj, rqst.alloc_size(), this);
}

size_t managed_pool_chunk::destroy(byte* ptr)
{
    using namespace collectors;

    ptr -= sizeof(traceable_object_meta);

    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(ptr);
    if (is_init(idx)) {
        traceable_object_meta* meta = managed_object::get_meta(ptr);
        const gc_type_meta* tmeta = meta->get_type_meta();
        if (tmeta) {
            tmeta->destroy(ptr);
        }
        set_init(idx, false);
        return m_cell_size;
    }
    return 0;
}

void managed_pool_chunk::move(byte* from, byte* to)
{
    using namespace collectors;

    managed_object obj_from(from);
    managed_object obj_to(to);

    memcpy(obj_to.meta(), obj_from.meta(), sizeof(traceable_object_meta));

    const gc_type_meta* from_meta = obj_from.meta()->get_type_meta();
    from_meta->move(from, to);
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
    assert(memory());
    return iterator(memory() + sizeof(collectors::traceable_object_meta), this);
}

managed_pool_chunk::iterator managed_pool_chunk::end()
{
    assert(memory());
    return iterator(memory() + size() + sizeof(collectors::traceable_object_meta), this);
}

bool managed_pool_chunk::is_init(size_t idx) const
{
    return m_init_bits.get(idx);
}

void managed_pool_chunk::set_init(size_t idx, bool init)
{
    m_init_bits.set(idx, init);
}

bool managed_pool_chunk::is_init(byte* ptr) const
{
    ptr -= sizeof(collectors::traceable_object_meta);
    return is_init(calc_cell_ind(ptr));
}

void managed_pool_chunk::set_init(byte* ptr, bool init)
{
    ptr -= sizeof(collectors::traceable_object_meta);
    set_init(calc_cell_ind(ptr), init);
}

bool managed_pool_chunk::get_mark(size_t idx) const
{
    return m_mark_bits.get(idx);
}

bool managed_pool_chunk::get_mark(byte* ptr) const
{
    ptr -= sizeof(collectors::traceable_object_meta);
    size_t ind = calc_cell_ind(ptr);
    return m_mark_bits.get(ind);
}

bool managed_pool_chunk::get_pin(size_t idx) const
{
    return m_pin_bits.get(idx);
}

bool managed_pool_chunk::get_pin(byte* ptr) const
{
    ptr -= sizeof(collectors::traceable_object_meta);
    size_t ind = calc_cell_ind(ptr);
    return m_pin_bits.get(ind);
}

void managed_pool_chunk::set_mark(size_t idx, bool mark)
{
    m_mark_bits.set(idx, mark);
}

void managed_pool_chunk::set_mark(byte* ptr, bool mark)
{
    ptr -= sizeof(collectors::traceable_object_meta);
    size_t ind = calc_cell_ind(ptr);
    m_mark_bits.set(ind, mark);
}

void managed_pool_chunk::set_pin(size_t idx, bool pin)
{
    m_pin_bits.set(idx, pin);
}

void managed_pool_chunk::set_pin(byte* ptr, bool pin)
{
    ptr -= sizeof(collectors::traceable_object_meta);
    size_t ind = calc_cell_ind(ptr);
    m_pin_bits.set(ind, pin);
}

size_t managed_pool_chunk::cell_size() const
{
    return m_cell_size;
}

size_t managed_pool_chunk::cell_size(byte* ptr) const
{
    return m_cell_size;
}

byte* managed_pool_chunk::cell_start(byte* ptr) const
{
    std::uintptr_t uintptr = reinterpret_cast<std::uintptr_t>(ptr);
    uintptr -= uintptr % cell_size(ptr);
    assert(uintptr % cell_size(ptr) == 0);
    return reinterpret_cast<byte*>(uintptr);
}

size_t managed_pool_chunk::object_count(byte* ptr) const
{
    return get_meta(ptr)->object_count();
}

const gc_type_meta* managed_pool_chunk::get_type_meta(byte* ptr) const
{
    return get_meta(ptr)->get_type_meta();
}

void managed_pool_chunk::set_type_meta(byte* ptr, const gc_type_meta* tmeta)
{
    get_meta(ptr)->set_type_meta(tmeta);
}

size_t managed_pool_chunk::calc_cell_ind(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return (ptr - memory()) / m_cell_size;
}

collectors::traceable_object_meta* managed_pool_chunk::get_meta(byte* ptr) const
{
    assert(contains(ptr));
//    assert(ptr == cell_start(ptr));
    return collectors::managed_object(ptr).meta();
}

}}}
