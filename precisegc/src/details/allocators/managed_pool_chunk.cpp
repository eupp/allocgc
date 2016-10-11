#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>

#include <cassert>
#include <limits>
#include <utility>

#include <libprecisegc/details/collectors/managed_object.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/utils/math.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

//managed_pool_chunk::managed_pool_chunk()
//    : m_chunk()
//    , m_cell_size(0)
//    , m_log2_cell_size(0)
//    , m_mask(0)
//{}

managed_pool_chunk::managed_pool_chunk(byte* chunk, size_t size, size_t cell_size)
    : m_chunk(chunk, size, cell_size)
    , m_cell_size(cell_size)
    , m_log2_cell_size(log2(cell_size))
    , m_mask(calc_mask(chunk, size, cell_size))
{
    collectors::memory_index::add_to_index(chunk, size, this);
}

managed_pool_chunk::~managed_pool_chunk()
{
    collectors::memory_index::remove_from_index(memory(), size());
}


byte* managed_pool_chunk::memory() const
{
    return m_chunk.get_mem();
}

size_t managed_pool_chunk::size() const
{
    return m_chunk.get_mem_size();
}

memory_descriptor* managed_pool_chunk::get_descriptor()
{
    return this;
}

bool managed_pool_chunk::all_unmarked() const noexcept
{
    return m_mark_bits.none();
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

managed_pool_chunk::iterator managed_pool_chunk::begin()
{
    assert(memory());
    return iterator(memory(), this);
}

managed_pool_chunk::iterator managed_pool_chunk::end()
{
    assert(memory());
    return iterator(memory() + size(), this);
}

managed_pool_chunk::memory_range_type managed_pool_chunk::memory_range()
{
    return memory_range_type(begin(), end());
}

bool managed_pool_chunk::get_mark(size_t i) const
{
    return m_mark_bits.get(i);
}

bool managed_pool_chunk::get_mark(byte* ptr) const
{
    size_t ind = calc_cell_ind(ptr);
    return m_mark_bits.get(ind);
}

bool managed_pool_chunk::get_pin(size_t i) const
{
    return m_pin_bits.get(i);
}

bool managed_pool_chunk::get_pin(byte* ptr) const
{
    size_t ind = calc_cell_ind(ptr);
    return m_pin_bits.get(ind);
}

void managed_pool_chunk::set_mark(size_t i, bool mark)
{
    m_mark_bits.set(i, mark);
}

void managed_pool_chunk::set_mark(byte* ptr, bool mark)
{
    size_t ind = calc_cell_ind(ptr);
    m_mark_bits.set(ind, mark);
}

void managed_pool_chunk::set_pin(size_t i, bool pin)
{
    m_pin_bits.set(i, pin);
}

void managed_pool_chunk::set_pin(byte* ptr, bool pin)
{
    size_t ind = calc_cell_ind(ptr);
    m_pin_bits.set(ind, pin);
}

size_t managed_pool_chunk::cell_size() const
{
    return m_cell_size;
}

byte* managed_pool_chunk::cell_start(byte* ptr) const
{
    uintptr uiptr = reinterpret_cast<uintptr>(ptr);
    uintptr res = (uiptr & m_mask);
    assert(res % m_cell_size == 0);
    return reinterpret_cast<byte*>(res);
}

void managed_pool_chunk::set_type_meta(byte* ptr, const gc_type_meta* tmeta)
{
    assert(m_chunk.contains(ptr));
//    assert(ptr == cell_start(ptr));
    collectors::managed_object(ptr).meta()->set_type_meta(tmeta);
}

managed_pool_chunk::uintptr managed_pool_chunk::calc_mask(byte* chunk,
                                                          size_t chunk_size,
                                                          size_t cell_size)
{
    size_t cell_size_bits = log2(cell_size);
    return std::numeric_limits<uintptr>::max() << cell_size_bits;
}

size_t managed_pool_chunk::calc_cell_ind(byte* ptr) const
{
    assert(memory() <= ptr && ptr < memory() + size());
    byte* cell_ptr = cell_start(ptr);
    assert((cell_ptr - memory()) % pow2(m_log2_cell_size) == 0);
    return (cell_ptr - memory()) >> m_log2_cell_size;
}

size_t managed_pool_chunk::get_log2_cell_size() const
{
    return m_log2_cell_size;
}

}}}
