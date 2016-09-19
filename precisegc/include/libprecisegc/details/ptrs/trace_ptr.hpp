#ifndef DIPLOMA_TRACE_PTR_HPP
#define DIPLOMA_TRACE_PTR_HPP

#include <cassert>

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_mark.hpp>

namespace precisegc { namespace details { namespace ptrs {

template<typename Functor>
void trace_ptr(object_meta* obj_meta, Functor&& f)
{
    assert(obj_meta);

    obj_meta->trace_children([&f] (gc_handle* handle) {
        managed_ptr mp(handle->rbarrier());
        if (mp && !mp.get_mark()) {
            mp.set_mark(true);
            object_meta* dbg = mp.get_meta();
            if (dbg == (object_meta*) 0x7ffff4dbd000) {
                logging::debug() << "trap!";
            }
            f(mp.get_meta());
        }
    });
}

}

}}

#endif //DIPLOMA_TRACE_PTR_HPP
