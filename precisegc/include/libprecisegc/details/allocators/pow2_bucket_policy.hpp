#ifndef DIPLOMA_POW2_BUCKET_POLICY_H
#define DIPLOMA_POW2_BUCKET_POLICY_H

#include <cstddef>
#include <cassert>
#include <cstdint>
#include <limits>

#include <libprecisegc/details/utils/math.hpp>

namespace precisegc { namespace details { namespace allocators {

template <size_t MIN_SIZE_BITS, size_t MAX_SIZE_BITS>
class pow2_bucket_policy
{
public:
    static const size_t BUCKET_COUNT = MAX_SIZE_BITS - MIN_SIZE_BITS + 1;

    // (1 << CHAR_BIT * sizeof(std::uint8_t))
    static_assert(BUCKET_COUNT <= std::numeric_limits<std::uint8_t>::max(), "Too many buckets");

    static size_t bucket(size_t size)
    {
        static bool init_flag = init_table();
        assert(size <= (1 << MAX_SIZE_BITS));
        return table[size];
    }

    static size_t bucket_size(size_t i)
    {
        assert(i < BUCKET_COUNT);
        return (size_t) 1 << (i + MIN_SIZE_BITS);
    }
private:
    static const size_t TABLE_SIZE = (1 << MAX_SIZE_BITS) + 1;

    static bool init_table()
    {
        for (size_t size = 1; size < TABLE_SIZE; ++size) {
            size_t i = (size == 1) ? 0 : msb(size - 1) + 1;
            size_t bucket = i < MIN_SIZE_BITS ? 0 : i - MIN_SIZE_BITS;
            assert(bucket <= std::numeric_limits<std::uint8_t>::max());
            table[size] = bucket;
        }
    }

    static std::uint8_t table[TABLE_SIZE];
};

template <size_t MIN_SIZE_BITS, size_t MAX_SIZE_BITS>
std::uint8_t pow2_bucket_policy<MIN_SIZE_BITS, MAX_SIZE_BITS>::table[pow2_bucket_policy::TABLE_SIZE] = {};

}}}

#endif //DIPLOMA_POW2_BUCKET_POLICY_H
