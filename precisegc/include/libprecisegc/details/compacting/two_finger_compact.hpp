#ifndef DIPLOMA_TWO_FINGER_COMPACT_HPP
#define DIPLOMA_TWO_FINGER_COMPACT_HPP

#include <cstddef>
#include <iterator>
#include <algorithm>

#include <libprecisegc/details/managed_ptr.hpp>

namespace precisegc { namespace details { namespace compacting {

template <typename Range, typename Forwarding>
size_t two_finger_compact(Range& rng, Forwarding& frwd, size_t obj_size)
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

}}}

#endif //DIPLOMA_TWO_FINGER_COMPACT_HPP
