#ifndef DIPLOMA_TWO_FINGER_COMPACT_HPP
#define DIPLOMA_TWO_FINGER_COMPACT_HPP

#include <cstddef>
#include <iterator>
#include <algorithm>

#include <libprecisegc/details/allocators/managed_memory_iterator.hpp>

namespace precisegc { namespace details { namespace compacting {

struct two_finger_compactor
{
    template <typename Range, typename Forwarding>
    size_t operator()(Range& rng, Forwarding& frwd) const
    {
        typedef typename Range::iterator iterator_t;
        typedef typename iterator_t::value_type value_t;
        typedef std::reverse_iterator<typename Range::iterator> reverse_iterator;

        if (rng.begin() == rng.end()) {
            return 0;
        }

        assert(std::all_of(rng.begin(), rng.end(),
                           [&rng] (const value_t& p) { return p.cell_size() == rng.begin()->cell_size(); }
        ));

        auto to = rng.begin();
        auto from = rng.end();
        size_t cell_size = to->cell_size();
        size_t copied_cnt = 0;
        while (from != to) {
            to = std::find_if(to, from, [](value_t cell_ptr) {
                return !cell_ptr.get_mark();
            });

            auto rev_from = std::find_if(reverse_iterator(from),
                                         reverse_iterator(to),
                                         [] (value_t cell_ptr) {
                                             return cell_ptr.get_mark() && !cell_ptr.get_pin();
                                         });

            from = rev_from.base();
            if (from != to) {
                --from;

                to->destroy();
                to->set_mark(true);

                from->move(*to);

                from->destroy();
                from->set_mark(false);

                frwd.create(from->get(), to->get(), cell_size);
                ++copied_cnt;
            }
        }
        return copied_cnt * cell_size;
    }
};

}}}

#endif //DIPLOMA_TWO_FINGER_COMPACT_HPP
