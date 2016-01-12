#ifndef DIPLOMA_UTIL_H
#define DIPLOMA_UTIL_H

#include <cstddef>
#include <climits>

namespace precisegc { namespace details {

/* returns log_2 if argument is a power of 2 */
inline size_t log_2(size_t n)
{
    assert((n & (n - 1)) != 0 || n == 0);
    size_t l = sizeof(size_t) * CHAR_BIT, r = 0;
    while (true) {
        int m = ((l - r) >> 1) + r;
        if (l == m || r == m) { return m; }
        if (((((size_t)1 << m) - 1) & n) == 0) { r = m; }
        else { l = m; }
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
