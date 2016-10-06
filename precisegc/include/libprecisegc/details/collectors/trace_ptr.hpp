#ifndef DIPLOMA_TRACE_PTR_HPP
#define DIPLOMA_TRACE_PTR_HPP

#include <cassert>

#include <libprecisegc/details/collectors/managed_object.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/collectors/indexed_managed_object.hpp>

namespace precisegc { namespace details { namespace collectors {

template<typename Functor>
void trace_ptr(managed_object obj, Functor&& f)
{
    assert(obj);

    obj.trace_children([&f] (gc_handle* handle) {
        indexed_managed_object idx_obj = indexed_managed_object::index_by_indirect_ptr(handle->rbarrier());
        if (idx_obj && !idx_obj.get_mark()) {
            idx_obj.set_mark(true);
            f(idx_obj.object());
        }
    });
}

}

}}

#endif //DIPLOMA_TRACE_PTR_HPP
