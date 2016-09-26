#ifndef DIPLOMA_FIX_PTRS_HPP
#define DIPLOMA_FIX_PTRS_HPP

#include <cstddef>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/object_meta.hpp>

namespace precisegc { namespace details { namespace compacting {

template <typename Iterator, typename Forwarding>
void fix_ptrs(const Iterator& first, const Iterator& last, const Forwarding& frwd, size_t obj_size)
{
    for (auto it = first; it != last; ++it) {
        if (!it->get_mark()) {
            continue;
        }
        byte* ptr = it->get();
        object_meta* obj_meta = object_meta::get_meta_ptr(ptr, obj_size);
        obj_meta->trace_children([&frwd] (gc_handle* handle) {
            frwd.forward(handle);
        });
    }
}

}}}

#endif //DIPLOMA_FIX_PTRS_HPP
