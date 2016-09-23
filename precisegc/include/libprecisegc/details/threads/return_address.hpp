#ifndef DIPLOMA_RETURN_ADDRESS_HPP
#define DIPLOMA_RETURN_ADDRESS_HPP

#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace threads {

inline byte* return_address()
{
    return reinterpret_cast<byte*>(__builtin_return_address(0));
}

inline byte* frame_address()
{
    return reinterpret_cast<byte*>(__builtin_frame_address(0));
}

}}}

#endif //DIPLOMA_RETURN_ADDRESS_HPP
