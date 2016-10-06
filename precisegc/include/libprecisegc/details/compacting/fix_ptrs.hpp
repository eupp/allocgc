#ifndef DIPLOMA_FIX_PTRS_HPP
#define DIPLOMA_FIX_PTRS_HPP

#include <cstddef>

#include <libprecisegc/details/gc_word.hpp>
#include <libprecisegc/details/collectors/managed_object.hpp>
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>

namespace precisegc { namespace details { namespace compacting {

template <typename Iterator, typename Forwarding>
void fix_ptrs(const Iterator& first, const Iterator& last, const Forwarding& frwd, size_t obj_size)
{
    using namespace collectors;

    for (auto it = first; it != last; ++it) {
        if (!it->get_mark()) {
            continue;
        }
        auto obj = managed_object(managed_object::get_object(it->get()));
        obj.trace_children([&frwd] (gc_word* handle) {
            frwd.forward(handle);
        });
    }
}

}}}

#endif //DIPLOMA_FIX_PTRS_HPP
