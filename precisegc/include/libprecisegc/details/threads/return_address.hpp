#ifndef DIPLOMA_RETURN_ADDRESS_HPP
#define DIPLOMA_RETURN_ADDRESS_HPP

#include <sys/resource.h>

#include <libprecisegc/gc_common.hpp>

namespace precisegc { namespace details { namespace threads {

inline byte* return_address()
{
    return reinterpret_cast<byte*>(__builtin_return_address(0));
}

inline byte* frame_address()
{
    return reinterpret_cast<byte*>(__builtin_frame_address(0));
}

inline size_t stack_maxsize()
{
    rlimit limit;
    getrlimit (RLIMIT_STACK, &limit);
    return limit.rlim_cur;
}

}}}

#endif //DIPLOMA_RETURN_ADDRESS_HPP
