#ifndef DIPLOMA_TRACE_PTR_HPP
#define DIPLOMA_TRACE_PTR_HPP

#include <cassert>

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_mark.hpp>

namespace precisegc { namespace details { namespace ptrs {

template<typename Functor>
void trace_ptr(managed_ptr p, Functor&& f)
{
    if (!p) {
        return;
    }

    object_meta* obj_meta = p.get_meta();
    size_t obj_count = obj_meta->object_count();
    const type_meta* cls_meta = obj_meta->get_type_meta();
    if (!cls_meta || cls_meta->is_plain_type()) {
        return;
    }

    size_t obj_size = cls_meta->type_size(); // sizeof array element
    auto offsets = cls_meta->offsets();
    byte* obj = p.get_cell_begin();
    size_t offsets_size = offsets.size();
    for (size_t i = 0; i < obj_count; i++) {
        for (size_t j = 0; j < offsets_size; j++) {
            gc_handle* pchild = reinterpret_cast<gc_handle*>((char*) obj + offsets[j]);
            managed_ptr child = managed_ptr(pchild->rbarrier());
            if (child && !child.get_mark()) {
                child.set_mark(true);
                f(child);
            }
        }
        obj += obj_size;
    }
}

}

}}

#endif //DIPLOMA_TRACE_PTR_HPP
