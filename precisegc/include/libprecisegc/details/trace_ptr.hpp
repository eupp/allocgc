#ifndef DIPLOMA_TRACE_PTR_HPP
#define DIPLOMA_TRACE_PTR_HPP

#include <cassert>

#include <libprecisegc/details/gc_untyped_ptr.h>
#include <libprecisegc/details/managed_ptr.h>
#include <libprecisegc/details/gc_mark.h>

namespace precisegc { namespace details {

template <typename Queue>
void trace_ptr(void* p, Queue& q)
{
    if (!p) {
        return;
    }

    managed_cell_ptr mp(managed_ptr((byte*) p), 0);

    assert(mp.is_live());
    if (mp.get_mark()) {
        return;
    }
    mp.set_mark(true);

    object_meta* obj_meta = mp.get_meta();
    const class_meta* cls_meta = obj_meta->get_class_meta();
    size_t obj_size = obj_meta->get_class_meta()->get_type_size(); // sizeof array element
    auto& offsets = cls_meta->get_offsets();

    if (offsets.empty()) {
        return;
    }

    byte* obj = mp.get_cell_begin();
    size_t obj_count = obj_meta->get_count();
    size_t offsets_size = offsets.size();
    for (size_t i = 0; i < obj_count; i++) {
        for (size_t j = 0; j < offsets_size; j++) {
            gc_untyped_ptr* pchild = (gc_untyped_ptr*) ((char *) obj + offsets[j]);
            void* child = pchild->get();
            if (child && !get_object_mark(child)) {
                q.push(child);
            }
        }
        obj += obj_size;
    }
}

}}

#endif //DIPLOMA_TRACE_PTR_HPP
