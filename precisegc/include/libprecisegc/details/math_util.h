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


// implementation is taken from http://stackoverflow.com/a/7767592/4676150
inline size_t msb(unsigned long long n)
{
    static const unsigned long long maskv[] = {
            0x000000007FFFFFFF,
            0x000000000000FFFF,
            0x00000000000000FF,
            0x000000000000000F,
            0x0000000000000003,
            0x0000000000000001
    };
    const unsigned long long *mask = maskv;

    assert(n != 0);

    size_t hi = 64;
    size_t lo = 0;

    do {
        size_t m = lo + (hi - lo) / 2;
        if ((n >> m) != 0) {
            lo = m;
        }
        else if ((n & (*mask << lo)) != 0) {
            hi = m;
        }

        mask++;
    } while (lo < hi - 1);

    return lo;
}

// not a best solution
inline size_t lsb(unsigned long long n)
{
    assert(n != 0);
    return msb(n & -n);
}

}}

#endif //DIPLOMA_UTIL_H
