#ifndef ALLOCGC_BITSET_HPP
#define ALLOCGC_BITSET_HPP

#include <cstddef>
#include <cstring>
#include <cassert>
#include <atomic>
#include <memory>
#include <limits>
#include <stdexcept>

#include <boost/integer/static_log2.hpp>
#include <boost/optional.hpp>

#include <liballocgc/details/utils/math.hpp>

namespace allocgc { namespace details { namespace utils {

namespace internals {

typedef unsigned long long ull;

class bitmap_block
{
public:
    bitmap_block()
        : m_val(0)
    {}

    bitmap_block(const bitmap_block&) = default;

    ull load() const
    {
        return m_val;
    }

    void store(ull val)
    {
        m_val = val;
    }

    bool compare_exchange(ull& expected, ull desired)
    {
        assert(m_val == expected);
        m_val = desired;
        return true;
    }

    void fetch_and(ull mask)
    {
        m_val &= mask;
    }

    void fetch_or(ull mask)
    {
        m_val |= mask;
    }

    void fetch_xor(ull mask)
    {
        m_val ^= mask;
    }
private:
    ull m_val;
};

class atomic_bitmap_block
{
public:
    atomic_bitmap_block()
        : m_val(0)
    {}

    atomic_bitmap_block(const atomic_bitmap_block& other)
        : m_val(other.load())
    {}

    ull load() const
    {
        return m_val.load(std::memory_order_relaxed);
    }

    void store(ull val)
    {
        m_val.store(val, std::memory_order_relaxed);
    }

    bool compare_exchange(ull& expected, ull desired)
    {
        return m_val.compare_exchange_weak(expected, desired, std::memory_order_relaxed);
    }

    void fetch_and(ull mask)
    {
        m_val.fetch_and(mask, std::memory_order_relaxed);
    }

    void fetch_or(ull mask)
    {
        m_val.fetch_or(mask, std::memory_order_relaxed);
    }

    void fetch_xor(ull mask)
    {
        m_val.fetch_xor(mask, std::memory_order_relaxed);
    }
private:
    std::atomic<ull> m_val;
};

template <typename Block>
class bitmap
{
public:
    explicit bitmap(size_t n)
        : m_size(block_num(n))
        , m_blocks(new Block[m_size])
    {
        reset_all();
    };

    template <typename Block_>
    bitmap(const bitmap& other)
        : m_size(other.m_size)
        , m_blocks(new Block[m_size])
    {
        for (size_t i = 0; i < m_size; ++i) {
            m_blocks[i].store(other.m_blocks[i].load());
        }
    }

    template <typename Block_>
    bitmap(bitmap&& other)
        : m_size(other.m_size)
        , m_blocks(std::move(other.m_blocks))
    {}

    bool get(size_t i) const
    {
        ull mask = ONE << bit_offset(i);
        return m_blocks[block_idx(i)].load() & mask;
    }

    void set(size_t i)
    {
        ull mask = ONE << bit_offset(i);
        m_blocks[block_idx(i)].fetch_or(mask);
    }

    void reset(size_t i)
    {
        ull mask = ONE << bit_offset(i);
        m_blocks[block_idx(i)].fetch_and(~mask);
    }

    void set(size_t i, bool val)
    {
        val ? set(i) : reset(i);
    }

    bool test_and_set(size_t i)
    {
        ull mask = ONE << bit_offset(i);
        size_t blk_idx = block_idx(i);
        ull old = m_blocks[blk_idx].load();
        while (!m_blocks[blk_idx].compare_exchange(old, old | mask)) {};
        return old & mask;
    }

    bitmap& set_all()
    {
        for (size_t i = 0; i < m_size; ++i) {
            m_blocks[i].store(MAX);
        }
        return *this;
    }

    bitmap& reset_all()
    {
        for (size_t i = 0; i < m_size; ++i) {
            m_blocks[i].store(ZERO);
        }
        return *this;
    }

    bool all() const
    {
        bool b = true;
        for (size_t i = 0; i < m_size; ++i) {
            b &= (m_blocks[i].load() == MAX);
        }
        return b;
    }

    bool any() const
    {
        bool b = false;
        for (size_t i = 0; i < m_size; ++i) {
            b |= (m_blocks[i].load() != 0);
        }
        return b;
    }

    bool none() const
    {
        bool b = true;
        for (size_t i = 0; i < m_size; ++i) {
            b &= (m_blocks[i].load() == 0);
        }
        return b;
    }

    size_t count() const
    {
        size_t cnt = 0;
        for (size_t i = 0; i < m_size; ++i) {
            ull n = m_blocks[i].load();
            while (n > 0) {
                ++cnt;
                n &= n - 1;
            }
        }
        return cnt;
    }

    size_t size() const
    {
        return m_size * BLOCK_SIZE;
    }

    bitmap& operator<<=(size_t pos)
    {
        size_t blk_shift  = block_idx(pos);
        size_t bits_shift = bit_offset(pos);
        size_t bits_carry = BLOCK_SIZE - bits_shift;

        for (size_t i = blk_shift + 1; i < m_size; ++i) {
            ull v = (m_blocks[i].load() << bits_shift) | (m_blocks[i - 1].load() >> bits_carry);
            m_blocks[i + blk_shift].store(v);
        }
        m_blocks[blk_shift].store(m_blocks[0].load() << bits_shift);
        for (size_t i = 0; i < blk_shift; ++i) {
            m_blocks[i].store(0);
        }

        return *this;
    }

    bitmap operator<<(size_t pos) const
    {
        return bitmap(*this) <<= pos;
    }

    bitmap& operator>>=(size_t pos)
    {
        size_t blk_shift  = block_idx(pos);
        size_t bits_shift = bit_offset(pos);
        size_t bits_carry = BLOCK_SIZE - bits_shift;

        if (blk_shift < m_size) {
            m_blocks[m_size - blk_shift - 1].store(m_blocks[m_size - 1].load() >> bits_shift);
            for (size_t i = 0; i < m_size - blk_shift - 1; ++i) {
                ull v = (m_blocks[i + blk_shift].load() >> bits_shift) | (m_blocks[i + blk_shift + 1].load() << bits_carry);
                m_blocks[i].store(v);
            }
        }
        for (size_t i = m_size - 1; i > m_size - blk_shift - 1; --i) {
            m_blocks[i].store(0);
        }

        return *this;
    }

    bitmap operator>>(size_t pos) const
    {
        return bitmap(*this) >>= pos;
    }

    bitmap& operator~()
    {
        size_t sz = m_size;
        for (size_t i = 0; i < sz; ++i) {
            m_blocks[i].store(~m_blocks[i].load());
        }
        return *this;
    }

    bitmap& operator|=(const bitmap& other)
    {
        assert(size() == other.size());
        size_t sz = m_size;
        for (size_t i = 0; i < sz; ++i) {
            m_blocks[i].fetch_or(other.m_blocks[i].load());
        }
        return *this;
    }

    bitmap& operator&=(const bitmap& other)
    {
        assert(size() == other.size());
        size_t sz = m_size;
        for (size_t i = 0; i < sz; ++i) {
            m_blocks[i].fetch_and(other.m_blocks[i].load());
        }
        return *this;
    }

    friend bitmap operator|(const bitmap& a, const bitmap& b)
    {
        return bitmap(a) |= b;
    }

    friend bitmap operator&(const bitmap& a, const bitmap& b)
    {
        return bitmap(a) &= b;
    }

    void swap(bitmap& other)
    {
        m_blocks.swap(other.m_blocks);
    }

    friend void swap(bitmap& a, bitmap& b)
    {
        a.swap(b);
    }

    template <typename Block_>
    friend class bitmap;
private:
    static const ull ZERO = 0;
    static const ull ONE  = 1;
    static const ull MAX  = std::numeric_limits<ull>::max();

    static const size_t BLOCK_SIZE = CHAR_BIT * sizeof(Block);
    static const size_t LOG2_BLOCK_SIZE = boost::static_log2<BLOCK_SIZE>::value;

    static_assert(check_pow2(BLOCK_SIZE), "bitmap block size must be power of two");

    static constexpr size_t block_num(size_t n)
    {
        return (n + BLOCK_SIZE - 1) / BLOCK_SIZE;
    }

    static constexpr size_t block_idx(size_t i)
    {
        return i >> LOG2_BLOCK_SIZE;
    }

    static constexpr size_t bit_offset(size_t i)
    {
        return i & (BLOCK_SIZE - 1);
    }

    size_t m_size;
    std::unique_ptr<Block[]> m_blocks;
};

}

using bitmap = internals::bitmap<internals::bitmap_block>;

using atomic_bitmap = internals::bitmap<internals::atomic_bitmap_block>;

}}}

#endif //ALLOCGC_BITSET_HPP
