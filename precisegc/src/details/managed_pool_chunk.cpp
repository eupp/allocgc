#include "managed_pool_chunk.h"

#include <utility>

#include "math_util.h"

namespace precisegc { namespace details {

managed_pool_chunk::managed_pool_chunk()
    : m_chunk()
    , m_descr(nullptr)
{}

managed_pool_chunk::managed_pool_chunk(byte* chunk, size_t size, size_t cell_size, memory_descriptor* descr)
    : m_chunk(chunk, size, cell_size)
    , m_descr(new memory_descriptor(this, chunk, size, cell_size))
{}

managed_pool_chunk::managed_pool_chunk(managed_pool_chunk&& other)
    : m_chunk()
    , m_descr(nullptr)
{
    using std::swap;
    lock_type lock = other.m_descr->lock();
    swap(m_descr, other.m_descr);
    swap(m_chunk, other.m_chunk);
    m_descr->set_pool_chunk(this);
}

managed_pool_chunk::~managed_pool_chunk()
{
    if (m_descr) {
        delete m_descr;
    }
}

ptr_manager managed_pool_chunk::allocate(size_t cell_size)
{
    lock_type lock = m_descr->lock();
    byte* raw_ptr = m_chunk.allocate(cell_size);
    return ptr_manager(managed_ptr(raw_ptr), m_descr, std::move(lock));
}

void managed_pool_chunk::deallocate(ptr_manager ptr, size_t cell_size)
{
//    lock_type lock = m_descr->lock();
    m_chunk.deallocate(ptr.get(), cell_size);
}

bool managed_pool_chunk::contains(byte* ptr) const
{
    return m_chunk.contains(ptr);
}

bool managed_pool_chunk::memory_available() const
{
    return m_chunk.memory_available();
}

bool managed_pool_chunk::empty(size_t cell_size) const
{
    return m_chunk.empty(cell_size);
}

void managed_pool_chunk::swap(managed_pool_chunk& other)
{
    using std::swap;
    swap(m_chunk, other.m_chunk);
    swap(m_descr, other.m_descr);
}

managed_pool_chunk::memory_descriptor::memory_descriptor(managed_pool_chunk* chunk_ptr,
                                                         byte* chunk_mem, size_t size, size_t cell_size)
        : m_chunk_ptr(chunk_ptr)
        , m_cell_size(cell_size)
        , m_mask(calculate_mask(chunk_mem, size, cell_size))
{
    managed_ptr::index(chunk_mem, size, this);
}

managed_pool_chunk::memory_descriptor::~memory_descriptor()
{
    managed_ptr::remove_index(m_chunk_ptr->m_chunk.get_mem(), m_chunk_ptr->m_chunk.get_mem_size());
}

managed_pool_chunk* managed_pool_chunk::memory_descriptor::get_pool_chunk() const
{
    return m_chunk_ptr;
}

void managed_pool_chunk::memory_descriptor::set_pool_chunk(managed_pool_chunk* pool_chunk)
{
    m_chunk_ptr = pool_chunk;
}

managed_pool_chunk::uintptr managed_pool_chunk::memory_descriptor::calculate_mask(byte* chunk, size_t chunk_size, size_t cell_size)
{
    size_t chunk_size_bits = log_2(chunk_size);
    size_t cell_size_bits = log_2(cell_size);
    size_t bit_diff = chunk_size_bits - cell_size_bits;
    uintptr ptr = reinterpret_cast<uintptr>(chunk);
    return (ptr | ((1 << bit_diff) - 1) << cell_size_bits);
}

}}