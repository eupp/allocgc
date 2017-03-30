#ifndef ALLOCGC_TO_STRING_HPP
#define ALLOCGC_TO_STRING_HPP

#include <sstream>
#include <string>

#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details { namespace utils {

inline std::string to_string(byte* address)
{
    std::stringstream ss;
    ss << (void*) address;
    return ss.str();
}

}}}

#endif //ALLOCGC_TO_STRING_HPP
