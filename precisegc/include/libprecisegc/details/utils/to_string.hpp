#ifndef DIPLOMA_TO_STRING_HPP
#define DIPLOMA_TO_STRING_HPP

#include <sstream>
#include <string>

#include <libprecisegc/details/types.h>

namespace precisegc { namespace details { namespace utils {

inline std::string to_string(byte* address)
{
    std::stringstream ss;
    ss << address;
    return ss.str();
}

}}}

#endif //DIPLOMA_TO_STRING_HPP
