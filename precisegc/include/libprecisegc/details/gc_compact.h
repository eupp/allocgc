#ifndef DIPLOMA_GC_COMPACT_H
#define DIPLOMA_GC_COMPACT_H

#include <iterator>
#include <algorithm>

#include "forwarding_list.h"
#include "thread_list.h"
#include "object_meta.h"
#include "gc_mark.h"
#include "../gc_ptr.h"
#include "../object.h"
#include "managed_ptr.h"

namespace precisegc { namespace details {

template <typename Range>
void two_finger_compact(Range& rng, size_t obj_size, forwarding_list& frwd)
{
    typedef std::reverse_iterator<typename Range::iterator> reverse_iterator;
    auto to = rng.begin();
    auto from = rng.end();
    while (from != to) {
        to = std::find_if(to, from,
                          [](managed_cell_ptr cell_ptr) { return !cell_ptr.get_mark() && !cell_ptr.get_pin(); });
        auto rev_from = std::find_if(reverse_iterator(from),
                                     reverse_iterator(to),
                                     [](managed_cell_ptr cell_ptr) { return cell_ptr.get_mark() && !cell_ptr.get_pin(); });
        from = rev_from.base();
        if (from != to) {
            --from;
            frwd.emplace_back(from->get(), to->get(), obj_size);
            from->set_mark(false);
            to->set_mark(true);
        }
    }
}

template <typename Range>
void sweep(Range& rng)
{
    for (managed_cell_ptr cell_ptr: rng) {
        if (cell_ptr.get_mark()) {
            cell_ptr.set_mark(false);
        } else {
            cell_ptr.sweep();
        }
    }
}

inline void fix_ptr(void* ptr, const forwarding_list& forwarding)
{
    void*& from = * ((void**) ptr);
    for (auto& frwd: forwarding) {
        if (from == frwd.from()) {
            from = frwd.to();
        }
    }
}

template <typename Iterator>
void fix_pointers(const Iterator& first, const Iterator& last, size_t obj_size, const forwarding_list& frwd)
{
    for (auto it = first; it != last; ++it) {
        void* ptr = *it;
        object_meta* obj_meta = object_meta::get_meta_ptr(ptr, obj_size);
        const class_meta* cls_meta = obj_meta->get_class_meta();
        if (cls_meta != nullptr) {
            size_t obj_size = cls_meta->get_type_size();
            auto& offsets = cls_meta->get_offsets();
            for (size_t i = 0; i < obj_meta->get_count(); ++i, ptr += obj_size) {
                for (size_t j = 0; j < offsets.size(); ++j) {
                    fix_ptr((void*) ptr + offsets[j], frwd);
                }
            }
        }
    }
}


inline void fix_roots(const forwarding_list& frwds)
{
    thread_list& tl = thread_list::instance();
    for (auto& handler: tl) {
        thread_handler* p_handler = &handler;
        StackMap *stack_ptr = p_handler->stack;
        for (StackElement* root = stack_ptr->begin(); root != NULL; root = root->next) {
//            printf("fix_root: from %p\n", get_pointed_to(root->addr));
//			fix_one_ptr(reinterpret_cast <void*> (*((size_t *)(root->addr))));
            void* new_place = nullptr;
            for (auto& frwd: frwds) {
                if (get_pointed_to(root->addr) == frwd.from()) {
                    new_place = frwd.to();
                }
            }
            if (new_place) {
                *(void * *)root->addr = new_place;
//				fixed_count++;
            }
//            printf("\t: to %p\n", get_pointed_to(root->addr));
        }
    }
}

}}

#endif //DIPLOMA_GC_COMPACT_H
