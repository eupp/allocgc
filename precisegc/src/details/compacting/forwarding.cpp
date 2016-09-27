#include <libprecisegc/details/compacting/forwarding.hpp>

#include <cassert>

#include <libprecisegc/details/object_meta.hpp>
#include <libprecisegc/details/gc_tagging.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace compacting {

void forwarding::create(byte* from, byte* to, size_t size)
{
    logging::debug() << "create forwarding: from " << (void*) from << " to " << (void*) to;

    move_cell(from, to, size);
    object_meta* meta = object_meta::get_meta_ptr(from, size);
    meta->set_forward_pointer(to);
}

void forwarding::forward(gc_handle* handle) const
{
    byte* tagged_ptr = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    byte* from = gc_tagging::clear(tagged_ptr);
    if (!from) {
        return;
    }

    object_meta* meta = object_meta::get_meta_ptr(tagged_ptr);
    if (meta->is_forwarded()) {
        byte* to = object_meta::get_object_ptr(meta->forward_pointer());
        if (gc_tagging::derived_bit(tagged_ptr)) {
            to += from - meta->get_object_begin();
        }
        logging::debug() << "fix ptr: from " << (void*) from << " to " << (void*) to;
        gc_handle_access::set<std::memory_order_relaxed>(*handle, to);
    }
}

void forwarding::move_cell(byte* from, byte* to, size_t size)
{
    assert(from);
    assert(to);
    memcpy(to, from, size);
}

}}}
