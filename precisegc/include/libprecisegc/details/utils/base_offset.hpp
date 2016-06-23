#ifndef DIPLOMA_BASE_OFFSET_HPP
#define DIPLOMA_BASE_OFFSET_HPP

#include <cstddef>

namespace precisegc { namespace details { namespace utils {

template <typename Base, typename Derived>
ptrdiff_t base_offset(Derived* d)
{
    return 0;
};

}}}

#endif //DIPLOMA_BASE_OFFSET_HPP
