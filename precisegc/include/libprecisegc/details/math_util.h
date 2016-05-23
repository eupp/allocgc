#ifndef DIPLOMA_UTIL_H
#define DIPLOMA_UTIL_H

#include <cstddef>
#include <climits>

namespace precisegc { namespace details {

constexpr bool check_pow2(size_t n)
{
    return (n != 0) && !(n & (n - 1));
}

constexpr size_t pow2(size_t n)
{
    return ((size_t) 1) << n;
}

// returns log2 if argument is a power of 2
inline size_t log2(size_t n)
{
    assert(check_pow2(n));
    size_t l = sizeof(size_t) * CHAR_BIT, r = 0;
    while (true) {
        size_t m = ((l - r) >> 1) + r;
        if (l == m || r == m) {
            return m;
        }
        if (((((size_t)1 << m) - 1) & n) == 0) {
            r = m;
        }
        else {
            l = m;
        }
    }
}

inline size_t msb(size_t n)
{
    size_t i = 0;
    while (n > 1) {
        n >>= 1;
        ++i;
    }
    return i;
}

}}

#endif //DIPLOMA_UTIL_H
