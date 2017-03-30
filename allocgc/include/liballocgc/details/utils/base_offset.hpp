#ifndef ALLOCGC_BASE_OFFSET_HPP
#define ALLOCGC_BASE_OFFSET_HPP

#include <cstddef>
#include <type_traits>

namespace allocgc { namespace details { namespace utils {

template <typename Base, typename Derived>
ptrdiff_t base_offset(Derived* d)
{
    static_assert(std::is_base_of<Base, Derived>::value, "Invalid type hierarchy");
    return reinterpret_cast<std::uintptr_t>(static_cast<Base*>(d)) - reinterpret_cast<std::uintptr_t>(d);
};

}}}

#endif //ALLOCGC_BASE_OFFSET_HPP
