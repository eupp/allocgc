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
    , m_descr(descr)
{
    descr->set_pool_chunk(this);
}

managed_pool_chunk::managed_pool_chunk(managed_pool_chunk&& other)
{
    lock_type lock = other.m_descr->lock();
    m_descr = other.m_descr;
    m_chunk = std::move(other.m_chunk);
    m_descr->set_pool_chunk(this);
}

managed_pool_chunk::~managed_pool_chunk()
{}

managed_pool_chunk& managed_pool_chunk::operator=(managed_pool_chunk&& chunk)
{
    chunk.swap(*this);
    return *this;
}

managed_cell_ptr managed_pool_chunk::allocate(size_t cell_size)
{
    lock_type lock = m_descr->lock();
    byte* raw_ptr = m_chunk.allocate(cell_size);
    return managed_cell_ptr(managed_ptr(raw_ptr), m_descr, std::move(lock));
}

void managed_pool_chunk::deallocate(managed_cell_ptr ptr, size_t cell_size)
{
//    lock_type lock = m_descr->lock();
    m_chunk.deallocate(ptr.get(), cell_size);
}

bool managed_pool_chunk::contains(byte* ptr) const noexcept
{
    return m_chunk.contains(ptr);
}

bool managed_pool_chunk::memory_available() const noexcept
{
    return m_chunk.memory_available();
}

bool managed_pool_chunk::empty(size_t cell_size) const noexcept
{
    return m_chunk.empty(cell_size);
}

byte* managed_pool_chunk::get_mem() const
{
    return m_chunk.get_mem();
}

size_t managed_pool_chunk::get_mem_size() const
{
    return m_chunk.get_mem_size();
}

managed_memory_descriptor* managed_pool_chunk::get_descriptor() const
{
    return m_descr;
}

managed_pool_chunk::range_type managed_pool_chunk::get_range() const
{
    assert(get_mem() && m_descr);
    byte* b = get_mem();
    byte* e = b + get_mem_size();
    auto lock = std::unique_lock<mutex>(m_descr->m_mutex, std::defer_lock_t());
    return range_type(iterator(b, m_descr), iterator(e, m_descr));
}

void managed_pool_chunk::swap(managed_pool_chunk& other)
{
    using std::swap;
    lock_type lock1;
    lock_type lock2;
    if (m_descr) {
        lock1 = m_descr->lock();
    }
    if (other.m_descr) {
        lock2 = other.m_descr->lock();
    }
    swap(m_chunk, other.m_chunk);
    swap(m_descr, other.m_descr);
    if (m_descr) {
        m_descr->set_pool_chunk(this);
    }
    if (other.m_descr) {
        other.m_descr->set_pool_chunk(&other);
    }
}

void swap(managed_pool_chunk& a, managed_pool_chunk& b)
{
    a.swap(b);
}

managed_pool_chunk::memory_descriptor::memory_descriptor(managed_pool_chunk* chunk_ptr,
                                                         byte* chunk_mem, size_t size, size_t cell_size)
    : m_chunk_ptr(chunk_ptr)
    , m_cell_size(cell_size)
    , m_mask(calc_mask(chunk_mem, size, cell_size))
{
    managed_ptr::index(chunk_mem, size, this);
}

managed_pool_chunk::memory_descriptor::~memory_descriptor()
{
    managed_ptr::remove_index(managed_ptr(m_chunk_ptr->m_chunk.get_mem()), m_chunk_ptr->m_chunk.get_mem_size());
}

managed_pool_chunk* managed_pool_chunk::memory_descriptor::get_pool_chunk() const
{
    return m_chunk_ptr;
}

void managed_pool_chunk::memory_descriptor::set_pool_chunk(managed_pool_chunk* pool_chunk)
{
    m_chunk_ptr = pool_chunk;
}

managed_pool_chunk::uintptr managed_pool_chunk::memory_descriptor::calc_mask(byte* chunk,
                                                                             size_t chunk_size,
                                                                             size_t cell_size)
{
    size_t chunk_size_bits = log_2(chunk_size);
    size_t cell_size_bits = log_2(cell_size);
    size_t bit_diff = chunk_size_bits - cell_size_bits;
    uintptr ptr = reinterpret_cast<uintptr>(chunk);
    return (ptr | ((1 << bit_diff) - 1) << cell_size_bits);
}

bool managed_pool_chunk::memory_descriptor::get_mark(byte* ptr)
{
    size_t ind = calc_cell_ind(ptr);
    return m_mark_bits[ind];
}

bool managed_pool_chunk::memory_descriptor::get_pin(byte* ptr)
{
    size_t ind = calc_cell_ind(ptr);
    return m_pin_bits[ind];
}

void managed_pool_chunk::memory_descriptor::set_mark(byte* ptr, bool mark)
{
    size_t ind = calc_cell_ind(ptr);
    m_mark_bits[ind] = mark;
}

void managed_pool_chunk::memory_descriptor::set_pin(byte* ptr, bool pin)
{
    size_t ind = calc_cell_ind(ptr);
    m_pin_bits[ind] = pin;
}

object_meta* managed_pool_chunk::memory_descriptor::get_cell_meta(byte* ptr)
{
    byte* cell_ptr = get_cell_begin(ptr);
    return object_meta::get_meta_ptr((void*) cell_ptr, m_cell_size);
}

byte* managed_pool_chunk::memory_descriptor::get_object_begin(byte* ptr)
{
    object_meta* meta = get_cell_meta(ptr);
    byte* cell_ptr = get_cell_begin(ptr);
    size_t obj_size = meta->get_class_meta()->get_type_size();
    size_t obj_ind = (ptr - cell_ptr) / obj_size;
    return cell_ptr + obj_ind * obj_size;
}

managed_memory_descriptor::lock_type managed_pool_chunk::memory_descriptor::lock()
{
    return std::unique_lock<mutex>(m_mutex);
}

managed_memory_descriptor::lock_type managed_pool_chunk::memory_descriptor::lock(std::defer_lock_t t)
{
    return std::unique_lock<mutex>(m_mutex, t);
}

managed_memory_descriptor::lock_type managed_pool_chunk::memory_descriptor::lock(std::try_to_lock_t t)
{
    return std::unique_lock<mutex>(m_mutex, t);
}

managed_memory_descriptor::lock_type managed_pool_chunk::memory_descriptor::lock(std::adopt_lock_t t)
{
    return std::unique_lock<mutex>(m_mutex, t);
}

size_t managed_pool_chunk::memory_descriptor::calc_cell_ind(byte* ptr) const
{
    byte* mem = m_chunk_ptr->get_mem();
    assert(mem <= ptr && ptr < mem + m_chunk_ptr->get_mem_size());
    byte* cell_ptr = get_cell_begin(ptr);
    assert((cell_ptr - mem) % m_cell_size == 0);
    return (cell_ptr - mem) / m_cell_size;
}

byte* managed_pool_chunk::memory_descriptor::get_cell_begin(byte* ptr) const
{
    uintptr uiptr = reinterpret_cast<uintptr>(ptr);
    return reinterpret_cast<byte*>(uiptr & m_mask);
}

size_t managed_pool_chunk::memory_descriptor::get_cell_size() const
{
    return m_cell_size;
}

managed_pool_chunk::iterator::iterator(byte* ptr, managed_pool_chunk::memory_descriptor* descr) noexcept
    : m_ptr(ptr)
    , m_descr(descr)
{}

bool managed_pool_chunk::iterator::equal(const managed_pool_chunk::iterator& other) const noexcept
{
    return m_ptr == other.m_ptr;
}

void managed_pool_chunk::iterator::increment() noexcept
{
    m_ptr += m_descr->get_cell_size();
}

void managed_pool_chunk::iterator::decrement() noexcept
{
    m_ptr -= m_descr->get_cell_size();
}

managed_cell_ptr managed_pool_chunk::iterator::operator*() const noexcept
{
    return managed_cell_ptr(managed_ptr(m_ptr), m_descr);
}

}}