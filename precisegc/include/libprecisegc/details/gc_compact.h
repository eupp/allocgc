#ifndef DIPLOMA_GC_COMPACT_H
#define DIPLOMA_GC_COMPACT_H

#include "forwarding_list.h"
#include "../object.h"

namespace precisegc { namespace details {

template <typename Iterator>
void two_finger_compact(const Iterator& first, const Iterator& last, size_t obj_size, forwarding_list& frwd)
{
    if (first == last) {
        return;
    }

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
        if (from != to) {
            frwd.emplace_back(*from, *to, obj_size);
            from = deallocate_it(from);
            if (from != to) {
                to++;
            }
        }
    }
    if (!from.is_marked()) {
        deallocate_it(from);
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
        size_t ptr = (size_t) *it;
        Object* obj = (Object*) (ptr + obj_size - sizeof(Object));
        if (obj->meta != nullptr) {
            size_t* meta = (size_t*) obj->meta;
            size_t obj_size = meta[0];
            size_t offsets_count = meta[1];
            for (size_t i = 0; i < obj->count; ++i, ptr += obj_size) {
                for (size_t j = 0; j < offsets_count; ++j) {
                    fix_ptr((void*) ptr + meta[2 + j], frwd);
                }
            }
        }
    }
}

}}

#endif //DIPLOMA_GC_COMPACT_H
