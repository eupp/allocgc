#ifndef DIPLOMA_GC_COMPACT_H
#define DIPLOMA_GC_COMPACT_H

#include <iterator>
#include <algorithm>

#include "forwarding.hpp"
#include "object_meta.hpp"
#include "gc_mark.hpp"
#include "libprecisegc/gc_ptr.hpp"
#include "managed_ptr.hpp"

#include "libprecisegc/details/threads/stack_map.hpp"

namespace precisegc { namespace details {

template <typename Range, typename Forwarding>
size_t two_finger_compact(Range& rng, size_t obj_size, Forwarding& frwd)
{
    typedef std::reverse_iterator<typename Range::iterator> reverse_iterator;
    size_t copied_cnt = 0;
    auto to = rng.begin();
    auto from = rng.end();
    while (from != to) {
        to = std::find_if(to, from, [](managed_ptr cell_ptr) {
                return !cell_ptr.get_mark();
        });

        auto rev_from = std::find_if(reverse_iterator(from),
                                     reverse_iterator(to),
                                     [] (managed_ptr cell_ptr) {
                                         return cell_ptr.get_mark() && !cell_ptr.get_pin();
        });

        from = rev_from.base();
        if (from != to) {
            --from;
            frwd.create(from->get(), to->get(), obj_size);
            from->set_mark(false);
            to->set_mark(true);
            ++copied_cnt;
        }
    }
    return copied_cnt;
}

//template <typename Range>
//size_t sweep(Range& rng)
//{
//    size_t sweep_cnt = 0;
//    for (managed_ptr cell_ptr: rng) {
//        if (cell_ptr.get_mark()) {
//            cell_ptr.set_mark(false);
//        } else if (cell_ptr.is_live() && !cell_ptr.get_pin()) {
//            cell_ptr.sweep();
//            sweep_cnt++;
//        }
//    }
//    return sweep_cnt;
//}


template <typename Iterator, typename Forwarding>
void fix_pointers(const Iterator& first, const Iterator& last, size_t obj_size, const Forwarding& frwd)
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

}}

#endif //DIPLOMA_GC_COMPACT_H
