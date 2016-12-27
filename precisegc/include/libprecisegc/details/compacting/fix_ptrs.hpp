#ifndef DIPLOMA_FIX_PTRS_HPP
#define DIPLOMA_FIX_PTRS_HPP

#include <cstddef>

#include <libprecisegc/details/gc_word.hpp>
#include <libprecisegc/details/gc_interface.hpp>

namespace precisegc { namespace details { namespace compacting {

template <typename Iterator, typename Forwarding>
void fix_ptrs(const Iterator& first, const Iterator& last, const Forwarding& frwd)
{
    for (auto it = first; it != last; ++it) {
        if (it->get_mark()) {
            it->trace(gc_trace_callback{[&frwd] (gc_word* handle) {
                frwd.forward(handle);
            }});
        }
    }
}

}}}

#endif //DIPLOMA_FIX_PTRS_HPP
