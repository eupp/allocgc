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

                auto to_obj = collectors::managed_object::make(to->get());
                const gc_type_meta* to_meta = to_obj.meta()->get_type_meta();
                to_meta->destroy(to_obj.object(), 0);
                to->set_mark(true);

                auto from_obj = collectors::managed_object::make(from->get());
                const gc_type_meta* from_meta = from_obj.meta()->get_type_meta();

                size_t from_obj_cnt = from_obj.meta()->object_count();

                memcpy(to->get(), from->get(), sizeof(collectors::traceable_object_meta));
                to_meta->move(from_obj.object(), to_obj.object(), from_obj_cnt);

                from_meta->destroy(from_obj.object(), from_obj_cnt);
                from->set_mark(false);
                from->set_dead();

                frwd.create(from->get(), to->get(), cell_size);

                ++copied_cnt;
            }
        }
        return copied_cnt * cell_size;
    }
};

}}}

#endif //DIPLOMA_TWO_FINGER_COMPACT_HPP
