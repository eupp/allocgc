#ifndef ALLOCGC_TWO_FINGER_COMPACT_HPP
#define ALLOCGC_TWO_FINGER_COMPACT_HPP

#include <cstddef>
#include <iterator>
#include <algorithm>

namespace allocgc { namespace details { namespace compacting {

struct two_finger_compactor
{
    template <typename Range, typename Forwarding>
    void operator()(Range& rng, Forwarding& frwd, gc_heap_stat& stat) const
    {
        typedef typename Range::iterator iterator_t;
        typedef typename iterator_t::value_type value_t;
        typedef std::reverse_iterator<typename Range::iterator> reverse_iterator;

        if (rng.begin() == rng.end()) {
            return;
        }

        assert(std::all_of(rng.begin(), rng.end(),
                           [&rng] (const value_t& p) { return p.cell_size() == rng.begin()->cell_size(); }
        ));

        auto to = rng.begin();
        auto from = rng.end();
        size_t cell_size = to->cell_size();
        size_t copied_cnt = 0;
        while (from != to) {
            to = std::find_if(to, from, [](value_t cell) {
                return cell.get_lifetime_tag() == gc_lifetime_tag::FREE ||
                       cell.get_lifetime_tag() == gc_lifetime_tag::GARBAGE;
            });

            if (to->get_lifetime_tag() == gc_lifetime_tag::GARBAGE) {
                stat.mem_freed += cell_size;
                to->finalize();
            }

            auto rev_from = std::find_if(reverse_iterator(from),
                                         reverse_iterator(to),
                                         [] (value_t cell) {
                                             return  cell.get_lifetime_tag() == gc_lifetime_tag::LIVE &&
                                                     !cell.get_pin();
                                         });

            from = rev_from.base();
            if (from != to) {
                --from;
                
                from->move(*to);
                from->finalize();
                frwd.create(from->get(), to->get());

//                stat.mem_freed  += cell_size;
                stat.mem_copied += cell_size;
            }
        }
    }
};

}}}

#endif //ALLOCGC_TWO_FINGER_COMPACT_HPP
