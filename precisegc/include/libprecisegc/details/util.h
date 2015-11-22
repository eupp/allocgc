#ifndef DIPLOMA_UTIL_H
#define DIPLOMA_UTIL_H

#include <cstddef>
#include <climits>

namespace precisegc { namespace details {

/* returns log_2 if argument is a power of 2
   otherwise --- -1 */
inline int log_2(size_t i)
{
    if ((i & (i -1)) != 0 || i == 0) { return -1; }
    int l = sizeof(size_t) * CHAR_BIT, r = 0;
    while (true) {
        int m = ((l - r) >> 1) + r;
        if (l == m || r == m) { return m; }
        if (((((size_t)1 << m) - 1) & i) == 0) { r = m; }
        else { l = m; }
    }
}

}}

#endif //DIPLOMA_UTIL_H
