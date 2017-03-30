#ifndef ALLOCGC_FIX_PTRS_HPP
#define ALLOCGC_FIX_PTRS_HPP

#include <cstddef>

#include <liballocgc/gc_handle.hpp>
#include <liballocgc/details/gc_interface.hpp>

namespace allocgc { namespace details { namespace compacting {

template <typename Iterator, typename Forwarding>
void fix_ptrs(const Iterator& first, const Iterator& last, const Forwarding& frwd)
{
    for (auto it = first; it != last; ++it) {
        if (it->get_mark()) {
            it->trace(gc_trace_callback{[&frwd] (gc_handle* handle) {
                frwd.forward(handle);
            }});
        }
    }
}

}}}

#endif //ALLOCGC_FIX_PTRS_HPP
