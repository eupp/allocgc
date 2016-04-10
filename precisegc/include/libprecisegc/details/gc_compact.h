#ifndef DIPLOMA_GC_COMPACT_H
#define DIPLOMA_GC_COMPACT_H

#include <iterator>
#include <algorithm>

#include "forwarding.h"
#include "thread_list.h"
#include "object_meta.h"
#include "gc_mark.h"
#include "../gc_ptr.h"
#include "managed_ptr.h"

namespace precisegc { namespace details {

template <typename Range, typename Forwarding>
void two_finger_compact(Range& rng, size_t obj_size, Forwarding& frwd)
{
    typedef std::reverse_iterator<typename Range::iterator> reverse_iterator;
    auto to = rng.begin();
    auto from = rng.end();
    while (from != to) {
        to = std::find_if(to, from, [](managed_cell_ptr cell_ptr) {
                return !cell_ptr.get_mark() && !cell_ptr.get_pin() && cell_ptr.is_live();
        });

        auto rev_from = std::find_if(reverse_iterator(from),
                                     reverse_iterator(to),
                                     [](managed_cell_ptr cell_ptr) {
                                         return cell_ptr.get_mark() && !cell_ptr.get_pin() && cell_ptr.is_live();
        });

        from = rev_from.base();
        if (from != to) {
            --from;
            frwd.create(from->get(), to->get(), obj_size);
            from->set_mark(false);
            to->set_mark(true);
        }
    }
}

template <typename Range>
size_t sweep(Range& rng)
{
    size_t sweep_cnt = 0;
    for (managed_cell_ptr cell_ptr: rng) {
        if (cell_ptr.get_mark()) {
            cell_ptr.set_mark(false);
        } else if (cell_ptr.is_live() && !cell_ptr.get_pin()) {
            cell_ptr.sweep();
            sweep_cnt++;
        }
    }
    return sweep_cnt;
}


template <typename Iterator, typename Forwarding>
void fix_pointers(const Iterator& first, const Iterator& last, size_t obj_size, const Forwarding& frwd)
{
    for (auto it = first; it != last; ++it) {
        if (!it->is_live()) {
            continue;
        }
        byte* ptr = it->get();
        object_meta* obj_meta = object_meta::get_meta_ptr(ptr, obj_size);
        const class_meta* cls_meta = obj_meta->get_class_meta();
        if (cls_meta != nullptr) {
            size_t obj_size = cls_meta->get_type_size();
            auto& offsets = cls_meta->get_offsets();
            size_t obj_count = obj_meta->get_count();
            size_t offsets_size = offsets.size();
            for (size_t i = 0; i < obj_count; ++i, ptr += obj_size) {
                for (size_t j = 0; j < offsets_size; ++j) {
                    frwd.forward((void*) ptr + offsets[j]);
                }
            }
        }
    }
}

template <typename Forwarding>
void fix_roots(const Forwarding& frwd)
{
    thread_list& tl = thread_list::instance();
    for (auto& handler: tl) {
        thread_handler* p_handler = &handler;
        StackMap *stack_ptr = p_handler->stack;
        for (StackElement* root = stack_ptr->begin(); root != NULL; root = root->next) {
            frwd.forward(root->addr);
        }
    }
}

}}

#endif //DIPLOMA_GC_COMPACT_H
