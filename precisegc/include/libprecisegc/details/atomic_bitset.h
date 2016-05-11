#ifndef DIPLOMA_ATOMIC_BITSET_H
#define DIPLOMA_ATOMIC_BITSET_H

#include <cstddef>
#include <atomic>
#include <limits>

#include "util.h"

namespace precisegc { namespace details {

class atomic_bitset : private noncopyable, private nonmovable
{
    typedef unsigned long long ull;
public:
    static const size_t SIZE = std::numeric_limits<ull>::digits;

    atomic_bitset();

    void set(std::memory_order order);
    void set(size_t i, std::memory_order order);
    void set(size_t i, bool value, std::memory_order order = std::memory_order_seq_cst);

    void reset(std::memory_order order = std::memory_order_seq_cst);
    void reset(size_t i, std::memory_order order = std::memory_order_seq_cst);

    bool test(size_t i, std::memory_order order = std::memory_order_seq_cst) const;
    bool operator[](size_t i) const;

    bool all(std::memory_order order = std::memory_order_seq_cst) const;
    bool none(std::memory_order order = std::memory_order_seq_cst) const;
private:
    static const ull BITS_SET = ~((ull) 0);

    std::atomic<ull> m_data;
};

}}

#endif //DIPLOMA_ATOMIC_BITSET_H
