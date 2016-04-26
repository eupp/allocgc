#include "managed_pool_chunk.h"

#include <utility>

#include "math_util.h"
#include "logging.h"

namespace precisegc { namespace details {

managed_pool_chunk::managed_pool_chunk(byte* chunk, size_t size, size_t cell_size)
    : m_chunk(chunk, size, cell_size)
    , m_cell_size(cell_size)
    , m_log2_cell_size(log_2(cell_size))
    , m_mask(calc_mask(chunk, size, cell_size))
{
    managed_ptr::index(chunk, size, this);
}

managed_pool_chunk::~managed_pool_chunk()
{
    managed_ptr::remove_index(managed_ptr(m_chunk.get_mem()), m_chunk.get_mem_size());
}

managed_cell_ptr managed_pool_chunk::allocate(size_t cell_size)
{
    assert(cell_size == get_cell_size());
    lock_type lk = lock();
    byte* raw_ptr = m_chunk.allocate(cell_size);
    size_t ind = calc_cell_ind(raw_ptr, get_log2_cell_size(), get_mem(), get_mem_size());
    m_alloc_bits[ind] = true;
    return managed_cell_ptr(managed_ptr(raw_ptr), cell_size, get_descriptor(), std::move(lk));
}

void managed_pool_chunk::deallocate(const managed_cell_ptr& ptr, size_t cell_size)
{
    assert(cell_size == get_cell_size());
    byte* raw_ptr = ptr.get();
    size_t ind = calc_cell_ind(raw_ptr, get_log2_cell_size(), get_mem(), get_mem_size());
    if (m_alloc_bits[ind]) {
        m_alloc_bits[ind] = false;
        m_chunk.deallocate(raw_ptr, cell_size);
    }
}

bool managed_pool_chunk::contains(const managed_cell_ptr& ptr) const noexcept
{
    return m_chunk.contains(ptr.get());
}

bool managed_pool_chunk::memory_available() const noexcept
{
    return m_chunk.memory_available();
}

bool managed_pool_chunk::empty(size_t cell_size) const noexcept
{
    return m_chunk.empty(cell_size);
}

bool managed_pool_chunk::is_dead() const noexcept
{
    return m_mark_bits.none() && m_pin_bits.none();
}

void managed_pool_chunk::reset_bits()
{
    m_mark_bits.reset();
    m_pin_bits.reset();
}

byte* managed_pool_chunk::get_mem() const
{
    return m_chunk.get_mem();
}

size_t managed_pool_chunk::get_mem_size() const
{
    return m_chunk.get_mem_size();
}

size_t managed_pool_chunk::get_cell_size() const
{
    return m_cell_size;
}

managed_memory_descriptor* managed_pool_chunk::get_descriptor()
{
    return this;
}

managed_pool_chunk::range_type managed_pool_chunk::get_range()
{
    assert(get_mem());
    byte* b = get_mem();
    byte* e = b + get_mem_size();
    return range_type(iterator(b, this), iterator(e, this));
}

void managed_pool_chunk::swap(managed_pool_chunk& other)
{
    using std::swap;
    lock_type lock1 = lock();
    lock_type lock2 = other.lock();
    swap(m_chunk, other.m_chunk);
    swap(m_alloc_bits, other.m_alloc_bits);
}

void swap(managed_pool_chunk& a, managed_pool_chunk& b)
{
    a.swap(b);
}

size_t managed_pool_chunk::calc_cell_ind(byte* ptr, size_t log2_cell_size, byte* base_ptr, size_t size)
{
    assert(base_ptr <= ptr && ptr < base_ptr + size);
    assert((ptr - base_ptr) % pow_2(log2_cell_size) == 0);
    return (ptr - base_ptr) >> log2_cell_size;
}

bool managed_pool_chunk::get_mark(byte* ptr)
{
    size_t ind = calc_cell_ind(ptr);
    return m_mark_bits[ind];
}

bool managed_pool_chunk::get_pin(byte* ptr)
{
    size_t ind = calc_cell_ind(ptr);
    return m_pin_bits[ind];
}

void managed_pool_chunk::set_mark(byte* ptr, bool mark)
{
    size_t ind = calc_cell_ind(ptr);
    m_mark_bits[ind] = mark;
}

void managed_pool_chunk::set_pin(byte* ptr, bool pin)
{
    size_t ind = calc_cell_ind(ptr);
    m_pin_bits[ind] = pin;
}

void managed_pool_chunk::sweep(byte* ptr)
{
    deallocate(managed_cell_ptr(managed_ptr(get_cell_begin(ptr)), 0), m_cell_size);
}

bool managed_pool_chunk::is_live(byte* ptr)
{
    size_t ind = calc_cell_ind(ptr);
    return  m_alloc_bits[ind];
}

object_meta* managed_pool_chunk::get_cell_meta(byte* ptr)
{
    byte* cell_ptr = get_cell_begin(ptr);
    return object_meta::get_meta_ptr((void*) cell_ptr, m_cell_size);
}

byte* managed_pool_chunk::get_object_begin(byte* ptr)
{
    object_meta* meta = get_cell_meta(ptr);
    byte* cell_ptr = get_cell_begin(ptr);
    size_t obj_size = meta->get_class_meta()->get_type_size();
    size_t obj_ind = (ptr - cell_ptr) / obj_size;
    return cell_ptr + obj_ind * obj_size;
}

managed_pool_chunk::lock_type managed_pool_chunk::lock()
{
    return std::unique_lock<mutex_type>(m_mutex);
}

managed_pool_chunk::lock_type managed_pool_chunk::lock(std::defer_lock_t t)
{
    return std::unique_lock<mutex_type>(m_mutex, t);
}

managed_pool_chunk::lock_type managed_pool_chunk::lock(std::try_to_lock_t t)
{
    return std::unique_lock<mutex_type>(m_mutex);
}

managed_pool_chunk::lock_type managed_pool_chunk::lock(std::adopt_lock_t t)
{
    return std::unique_lock<mutex_type>(m_mutex, t);
}

managed_pool_chunk::uintptr managed_pool_chunk::calc_mask(byte* chunk,
                                                          size_t chunk_size,
                                                          size_t cell_size)
{
    size_t chunk_size_bits = log_2(chunk_size);
    size_t cell_size_bits = log_2(cell_size);
    size_t bit_diff = chunk_size_bits - cell_size_bits;
    uintptr ptr = reinterpret_cast<uintptr>(chunk);
    return (ptr | (((1 << bit_diff) - 1) << cell_size_bits));
}

size_t managed_pool_chunk::calc_cell_ind(byte* ptr)
{
    return managed_pool_chunk::calc_cell_ind(get_cell_begin(ptr),
                                             m_log2_cell_size,
                                             get_mem(),
                                             get_mem_size());
}

byte* managed_pool_chunk::get_cell_begin(byte* ptr)
{
    uintptr uiptr = reinterpret_cast<uintptr>(ptr);
    uintptr res = (uiptr & m_mask);
    assert(res % m_cell_size == 0);
    return reinterpret_cast<byte*>(res);
}

size_t managed_pool_chunk::get_log2_cell_size() const
{
    return m_log2_cell_size;
}

managed_pool_chunk::iterator::iterator() noexcept
    : m_ptr(nullptr)
    , m_chunk(nullptr)
{}

managed_pool_chunk::iterator::iterator(byte* ptr, managed_pool_chunk* descr) noexcept
    : m_ptr(ptr)
    , m_chunk(descr)
{}

bool managed_pool_chunk::iterator::equal(const managed_pool_chunk::iterator& other) const noexcept
{
    return m_ptr == other.m_ptr;
}

void managed_pool_chunk::iterator::increment() noexcept
{
    m_ptr += m_chunk->get_cell_size();
}

void managed_pool_chunk::iterator::decrement() noexcept
{
    m_ptr -= m_chunk->get_cell_size();
}

managed_cell_ptr managed_pool_chunk::iterator::operator*() const noexcept
{
    return managed_cell_ptr(managed_ptr(m_ptr), 0, m_chunk->get_descriptor());
}

managed_pool_chunk::iterator::proxy managed_pool_chunk::iterator::operator->() const noexcept
{
    return proxy(managed_cell_ptr(managed_ptr(m_ptr), 0, m_chunk->get_descriptor()));
}

}}