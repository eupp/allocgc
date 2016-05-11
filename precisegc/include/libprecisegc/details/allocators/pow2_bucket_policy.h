#ifndef DIPLOMA_POW2_BUCKET_POLICY_H
#define DIPLOMA_POW2_BUCKET_POLICY_H

#include <cstddef>
#include <cassert>

#include "../math_util.h"

namespace precisegc { namespace details { namespace allocators {

template <size_t MIN_SIZE_BITS, size_t MAX_SIZE_BITS>
class pow2_bucket_policy
{
public:
    static const size_t BUCKET_COUNT = MAX_SIZE_BITS - MIN_SIZE_BITS + 1;

    static size_t bucket(size_t size)
    {
        size_t i = (size == 1) ? 0 : msb(size - 1) + 1;
        assert(i <= MAX_SIZE_BITS);
        return i < MIN_SIZE_BITS ? 0 : i - MIN_SIZE_BITS;
    }

    static size_t bucket_size(size_t i)
    {
        assert(i < BUCKET_COUNT);
        return (size_t) 1 << (i + MIN_SIZE_BITS);
    }
};

}}}

#endif //DIPLOMA_POW2_BUCKET_POLICY_H
