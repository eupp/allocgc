#ifndef DIPLOMA_FORWARDING_HPP
#define DIPLOMA_FORWARDING_HPP

#include <cstddef>

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace compacting {

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

#endif //DIPLOMA_FORWARDING_HPP
