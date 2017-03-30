#ifndef ALLOCGC_FORWARDING_HPP
#define ALLOCGC_FORWARDING_HPP

#include <cstddef>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/gc_handle.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace compacting {

class forwarding
{
public:
    forwarding() = default;
    forwarding(const forwarding&) = default;
    forwarding(forwarding&&) = default;

    void create(byte* from, byte* to);
    void forward(gc_handle* handle) const;
};

}}}

#endif //ALLOCGC_FORWARDING_HPP
