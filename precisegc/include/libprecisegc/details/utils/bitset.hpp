#ifndef DIPLOMA_BITSET_HPP
#define DIPLOMA_BITSET_HPP

#include <cstddef>
#include <cstring>
#include <atomic>
#include <array>
#include <limits>
#include <stdexcept>

#include <boost/integer/static_log2.hpp>
#include <boost/optional.hpp>

#include <libprecisegc/details/utils/math.hpp>
#include <iostream>

namespace precisegc { namespace details { namespace utils {

namespace internals {

typedef unsigned long long ull;

class bitset_block
{
public:
    bitset_block()
        : m_val(0)
    {}

    bitset_block(const bitset_block&) = default;

    ull load() const
    {
        return m_val;
    }
    void store(ull val)
    {
        m_val = val;
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

class sync_bitset_block
{
public:
    sync_bitset_block()
        : m_val(0)
    {}

    sync_bitset_block(const sync_bitset_block& other)
        : m_val(other.load())
    {}

    ull load() const
    {
        return m_val.load(std::memory_order_acquire);
    }

    void store(ull val)
    {
        m_val.store(val, std::memory_order_release);
    }

    void fetch_and(ull mask)
    {
        m_val.fetch_and(mask, std::memory_order_acq_rel);
    }

    void fetch_or(ull mask)
    {
        m_val.fetch_or(mask, std::memory_order_acq_rel);
    }

    void fetch_xor(ull mask)
    {
        m_val.fetch_xor(mask, std::memory_order_acq_rel);
    }
private:
    std::atomic<ull> m_val;
};

template <size_t N, typename Block>
class bitset
{
    static const size_t BLOCK_SIZE = CHAR_BIT * sizeof(Block);
    static const size_t LOG2_BLOCK_SIZE = boost::static_log2<BLOCK_SIZE>::value;
    static const size_t BLOCK_CNT = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;

    static_assert(check_pow2(BLOCK_SIZE), "bitset block size must be power of two");
public:
    bitset() = default;
    bitset(const bitset& other) = default;

    template <typename Block_>
    bitset(const bitset<N, Block_>& other)
    {
        for (size_t i = 0; i < BLOCK_CNT; ++i) {
            m_blocks[i].store(other.m_blocks[i].load());
        }
    }

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

    bitset& set_all()
    {
        for (auto& block: m_blocks) {
            block.store(MAX);
        }
        return *this;
    }

    bitset& reset_all()
    {
        for (auto& block: m_blocks) {
            block.store(ZERO);
        }
        return *this;
    }

    bool all() const
    {
        bool b = true;
        for (auto& block: m_blocks) {
            b &= (block.load() == MAX);
        }
        return b;
    }

    bool any() const
    {
        bool b = false;
        for (auto& block: m_blocks) {
            b |= (block.load() != 0);
        }
        return b;
    }

    bool none() const
    {
        bool b = true;
        for (auto& block: m_blocks) {
            b &= (block.load() == 0);
        }
        return b;
    }

    size_t count() const
    {
        size_t cnt = 0;
        for (auto& block: m_blocks) {
            ull n = block.load();
            while (n > 0) {
                ++cnt;
                n &= n - 1;
            }
        }
        return cnt;
    }

    size_t size() const
    {
        return N;
    }

    boost::optional<size_t> most_significant_bit() const
    {
        for (size_t i = BLOCK_CNT - 1; i > 0; --i) {
            ull val = m_blocks[i].load();
            if (val != 0) {
                return boost::make_optional(msb(val) + i * BLOCK_SIZE);
            }
        }
        ull val = m_blocks[0].load();
        return val != 0 ? boost::make_optional(msb(val)) : boost::optional<size_t>();
    }

    boost::optional<size_t> least_significant_bit() const
    {
        for (size_t i = 0; i < BLOCK_CNT - 1; ++i) {
            ull val = m_blocks[i].load();
            if (val != 0) {
                return boost::make_optional(lsb(val) + i * BLOCK_SIZE);
            }
        }
        ull val = m_blocks[BLOCK_CNT - 1].load();
        return val != 0 ? boost::make_optional(lsb(val) + (BLOCK_CNT - 1) * BLOCK_SIZE) : boost::optional<size_t>();
    }

    bitset& operator<<=(size_t pos)
    {
        size_t blk_shift  = block_idx(pos);
        size_t bits_shift = bit_offset(pos);
        size_t bits_carry = BLOCK_SIZE - bits_shift;

        for (size_t i = blk_shift + 1; i < BLOCK_CNT; ++i) {
            ull v = (m_blocks[i].load() << bits_shift) | (m_blocks[i - 1].load() >> bits_carry);
            m_blocks[i + blk_shift].store(v);
        }
        m_blocks[blk_shift].store(m_blocks[0].load() << bits_shift);
        for (size_t i = 0; i < blk_shift; ++i) {
            m_blocks[i].store(0);
        }

        return *this;
    }

    bitset operator<<(size_t pos) const
    {
        return bitset(*this) <<= pos;
    }

    bitset& operator>>=(size_t pos)
    {
        size_t blk_shift  = block_idx(pos);
        size_t bits_shift = bit_offset(pos);
        size_t bits_carry = BLOCK_SIZE - bits_shift;

        if (blk_shift < BLOCK_CNT) {
            m_blocks[BLOCK_CNT - blk_shift - 1].store(m_blocks[BLOCK_CNT - 1].load() >> bits_shift);
            for (size_t i = 0; i < BLOCK_CNT - blk_shift - 1; ++i) {
                ull v = (m_blocks[i + blk_shift].load() >> bits_shift) | (m_blocks[i + blk_shift + 1].load() << bits_carry);
                m_blocks[i].store(v);
            }
        }
        for (size_t i = BLOCK_CNT - 1; i > BLOCK_CNT - blk_shift - 1; --i) {
            m_blocks[i].store(0);
        }

        return *this;
    }

    bitset operator>>(size_t pos) const
    {
        return bitset(*this) >>= pos;
    }

    bitset& operator~()
    {
        for (size_t i = 0; i < BLOCK_CNT; ++i) {
            m_blocks[i].store(~m_blocks[i].load());
        }
        return *this;
    }

    bitset& operator|=(const bitset& other)
    {
        for (size_t i = 0; i < BLOCK_CNT; ++i) {
            m_blocks[i].fetch_or(other.m_blocks[i].load());
        }
        return *this;
    }

    bitset& operator&=(const bitset& other)
    {
        for (size_t i = 0; i < BLOCK_CNT; ++i) {
            m_blocks[i].fetch_and(other.m_blocks[i].load());
        }
        return *this;
    }

    friend bitset operator|(const bitset& a, const bitset& b)
    {
        return bitset(a) |= b;
    }

    friend bitset operator&(const bitset& a, const bitset& b)
    {
        return bitset(a) &= b;
    }

    void swap(bitset& other)
    {
        for (size_t i = 0; i < BLOCK_CNT; ++i) {
            ull tmp = m_blocks[i].load();
            m_blocks[i].store(other.m_blocks[i].load());
            other.m_blocks[i].store(tmp);
        }
    }

    friend void swap(bitset& a, bitset& b)
    {
        a.swap(b);
    }

    template <size_t N_, typename Block_>
    friend class bitset;
private:
    static const ull ZERO = 0;
    static const ull ONE  = 1;
    static const ull MAX  = std::numeric_limits<ull>::max();

    static constexpr size_t block_idx(size_t i)
    {
        return i >> LOG2_BLOCK_SIZE;
    }

    static constexpr size_t bit_offset(size_t i)
    {
        return i & (BLOCK_SIZE - 1);
    }

    std::array<Block, BLOCK_CNT> m_blocks;
};

}

template <size_t N>
using bitset = internals::bitset<N, internals::bitset_block>;

template <size_t N>
using sync_bitset = internals::bitset<N, internals::sync_bitset_block>;

}}}

#endif //DIPLOMA_BITSET_HPP
