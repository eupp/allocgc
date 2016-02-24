#ifndef DIPLOMA_GC_COMPACT_H
#define DIPLOMA_GC_COMPACT_H

#include "thread_list.h"
#include "object_meta.h"
#include "gc_mark.h"
#include "../gc_ptr.h"
#include "../object.h"

namespace precisegc { namespace details {

template <typename Iterator, typename Forwarding>
size_t two_finger_compact(const Iterator& first, const Iterator& last, size_t obj_size, Forwarding& frwd)
{
    if (first == last) {
        return 0;
    }

    size_t compacted_size = 0;

    auto deallocate_it = [](Iterator it) {
        auto tmp = it;
        --it;
        tmp.deallocate();
        return it;
    };

    auto from = last;
    auto to = first;
    --from;
    while (from != to) {
        while (!from.is_marked() && from != to) {
            if (from.is_pinned()) {
                --from;
            } else {
                from = deallocate_it(from);
            }
        }
        if (from.is_pinned() && from != to) {
            --from;
            continue;
        }
        while (to.is_marked() && from != to) {
            ++to;
        }
        if (to.is_pinned() && from != to) {
            ++to;
            continue;
        }
        if (from != to) {
            frwd.create(*from, *to, obj_size);
            compacted_size += obj_size;
            from = deallocate_it(from);
            if (from != to) {
                to++;
            }
        }
    }
    if (!from.is_marked()) {
        from.deallocate();
    }

    return compacted_size;
}


template <typename Iterator, typename Forwarding>
void fix_pointers(const Iterator& first, const Iterator& last, size_t obj_size, const Forwarding& frwd)
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
