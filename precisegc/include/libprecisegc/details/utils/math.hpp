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


// implementation is taken from http://stackoverflow.com/a/11398748/4676150
inline size_t msb(unsigned long long n)
{
    static const std::int8_t tab64[64] = {
            63,  0, 58,  1, 59, 47, 53,  2,
            60, 39, 48, 27, 54, 33, 42,  3,
            61, 51, 37, 40, 49, 18, 28, 20,
            55, 30, 34, 11, 43, 14, 22,  4,
            62, 57, 46, 52, 38, 26, 32, 41,
            50, 36, 17, 19, 29, 10, 13, 21,
            56, 45, 25, 31, 35, 16,  9, 12,
            44, 24, 15,  8, 23,  7,  6,  5
    };

    assert(n != 0);

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return tab64[((uint64_t)((n - (n >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}

/**
 * implementation is taken from http://chessprogramming.wikispaces.com/BitScan
 *
 * bitScanForward
 * @author Martin Läuter (1997)
 *         Charles E. Leiserson
 *         Harald Prokop
 *         Keith H. Randall
 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
inline size_t lsb(unsigned long long n)
{
    static const size_t index64[64] = {
            0,  1, 48,  2, 57, 49, 28,  3,
            61, 58, 50, 42, 38, 29, 17,  4,
            62, 55, 59, 36, 53, 51, 43, 22,
            45, 39, 33, 30, 24, 18, 12,  5,
            63, 47, 56, 27, 60, 41, 37, 16,
            54, 35, 52, 21, 44, 32, 23, 11,
            46, 26, 40, 15, 34, 20, 31, 10,
            25, 14, 19,  9, 13,  8,  7,  6
    };

    const unsigned long long debruijn64 = 0x03f79d71b4cb0a89ULL;
    assert (n != 0);
    return index64[((n & -n) * debruijn64) >> 58];
}

}}

#endif //DIPLOMA_UTIL_H
