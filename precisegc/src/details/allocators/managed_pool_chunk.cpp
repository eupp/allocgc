#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>

#include <cassert>
#include <limits>
#include <utility>

#include <libprecisegc/details/utils/math.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

managed_pool_chunk::managed_pool_chunk()
    : m_chunk()
    , m_cell_size(0)
    , m_log2_cell_size(0)
    , m_mask(0)
{}

managed_pool_chunk::managed_pool_chunk(byte* chunk, size_t size, size_t cell_size)
    : m_chunk(chunk, size, cell_size)
    , m_cell_size(cell_size)
    , m_log2_cell_size(log2(cell_size))
    , m_mask(calc_mask(chunk, size, cell_size))
{
    managed_ptr::add_to_index(chunk, size, this);
}

managed_pool_chunk::~managed_pool_chunk()
{
    managed_ptr::remove_from_index(get_mem(), get_mem_size());
}

managed_pool_chunk::pointer_type managed_pool_chunk::allocate(size_t size)
{
    assert(size == cell_size());
    byte* raw_ptr = nullptr;
    if (m_chunk.memory_available()) {
        raw_ptr = m_chunk.allocate(size);
    } else {
        assert(!m_freelist.empty());
        raw_ptr = m_freelist.allocate(size);
        memset(raw_ptr, 0, m_cell_size);
    }
    return managed_ptr(raw_ptr, this);
}

void managed_pool_chunk::deallocate(const pointer_type& ptr, size_t size)
{
    assert(size == cell_size());
    deallocate(ptr.get(), size);
}

void managed_pool_chunk::deallocate(byte* ptr, size_t size)
{
    assert(size == cell_size());
    m_chunk.deallocate(ptr, size);
}

bool managed_pool_chunk::contains(const pointer_type& ptr) const noexcept
{
    return m_chunk.contains(ptr.get());
}

bool managed_pool_chunk::memory_available() const noexcept
{
    return m_chunk.memory_available() || !m_freelist.empty();
}

bool managed_pool_chunk::empty() const noexcept
{
    return m_mark_bits.none();
}

byte* managed_pool_chunk::get_mem() const
{
    return m_chunk.get_mem();
}

size_t managed_pool_chunk::get_mem_size() const
{
    return m_chunk.get_mem_size();
}

memory_descriptor* managed_pool_chunk::get_descriptor()
{
    return this;
}

size_t managed_pool_chunk::sweep()
{
    m_freelist.reset();

    size_t freed = 0;
    size_t i = 0;
    byte* mem_end = m_chunk.get_top();
    for (byte *it = get_mem(); it < mem_end; it += m_cell_size, ++i) {
        if (!m_mark_bits.get(i)) {
            memset(it, 0, m_cell_size);
            m_freelist.deallocate(it, m_cell_size);
            freed += m_cell_size;
        }
    }
    return freed;
}

void managed_pool_chunk::unmark()
{
    m_mark_bits.reset_all();
    m_pin_bits.reset_all();
}

managed_pool_chunk::iterator managed_pool_chunk::begin()
{
    assert(get_mem());
    return iterator(get_mem(), this);
}

managed_pool_chunk::iterator managed_pool_chunk::end()
{
    assert(get_mem());
    return iterator(get_mem() + get_mem_size(), this);
}

managed_pool_chunk::range_type managed_pool_chunk::get_range()
{
    return range_type(begin(), end());
}

bool managed_pool_chunk::get_mark(byte* ptr) const
{
    size_t ind = calc_cell_ind(ptr);
    return m_mark_bits.get(ind);
}

bool managed_pool_chunk::get_pin(byte* ptr) const
{
    size_t ind = calc_cell_ind(ptr);
    return m_pin_bits.get(ind);
}

void managed_pool_chunk::set_mark(byte* ptr, bool mark)
{
    size_t ind = calc_cell_ind(ptr);
    m_mark_bits.set(ind, mark);
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

object_meta* managed_pool_chunk::get_cell_meta(byte* ptr) const
{
    byte* cell_ptr = get_cell_begin(ptr);
    return object_meta::get_meta_ptr((void*) cell_ptr, m_cell_size);
}

byte* managed_pool_chunk::get_cell_begin(byte* ptr) const
{
    uintptr uiptr = reinterpret_cast<uintptr>(ptr);
    uintptr res = (uiptr & m_mask);
    assert(res % m_cell_size == 0);
    return reinterpret_cast<byte*>(res);
}

byte* managed_pool_chunk::get_obj_begin(byte* ptr) const
{
    object_meta* meta = get_cell_meta(ptr);
    byte* cell_ptr = get_cell_begin(ptr);
    size_t obj_size = meta->get_type_meta()->type_size();
    size_t obj_ind = (ptr - cell_ptr) / obj_size;
    return cell_ptr + obj_ind * obj_size;
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
    assert(get_mem() <= ptr && ptr < get_mem() + get_mem_size());
    byte* cell_ptr = get_cell_begin(ptr);
    assert((cell_ptr - get_mem()) % pow2(m_log2_cell_size) == 0);
    return (cell_ptr - get_mem()) >> m_log2_cell_size;
}

size_t managed_pool_chunk::get_log2_cell_size() const
{
    return m_log2_cell_size;
}

}}}
