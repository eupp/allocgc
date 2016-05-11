#include "atomic_bitset.h"

#include <cassert>

namespace precisegc { namespace details {

atomic_bitset::atomic_bitset()
    : m_data(0)
{
    assert(m_data.is_lock_free());
}

void atomic_bitset::set(std::memory_order order)
{
    m_data.store(BITS_SET, order);
}

void atomic_bitset::set(size_t i, std::memory_order order)
{
    ull mask = (ull) 1 << i;
    m_data.fetch_or(mask, order);
}

void atomic_bitset::set(size_t i, bool value, std::memory_order order)
{
    value ? set(i, order) : reset(i, order);
}

void atomic_bitset::reset(std::memory_order order)
{
    m_data.store((ull) 0, order);
}

void atomic_bitset::reset(size_t i, std::memory_order order)
{
    ull mask = (ull) 1 << i;
    m_data.fetch_and(~mask, order);
}

bool atomic_bitset::test(size_t i, std::memory_order order) const
{
    ull mask = (ull) 1 << i;
    return m_data.load(order) & mask;
}

bool atomic_bitset::operator[](size_t i) const
{
    return test(i);
}

bool atomic_bitset::all(std::memory_order order) const
{
    return (m_data.load(order) ^ BITS_SET) == (ull) 0;
}

bool atomic_bitset::none(std::memory_order order) const
{
    return m_data.load(order) == (ull) 0;
}


}}

