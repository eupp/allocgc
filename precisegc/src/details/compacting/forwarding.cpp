#include <libprecisegc/details/compacting/forwarding.hpp>

#include <cassert>

#include <libprecisegc/details/collectors/dptr_storage.hpp>
#include <libprecisegc/details/collectors/managed_object.hpp>
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace compacting {

void forwarding::create(byte* from, byte* to, size_t size)
{
    using namespace collectors;

    logging::debug() << "create forwarding: from " << (void*) from << " to " << (void*) to;

    move_cell(from, to, size);
    traceable_object_meta* meta = managed_object::get_meta(from);
    meta->set_forward_pointer(to);
}

void forwarding::forward(gc_word* handle) const
{
    using namespace collectors;

    byte* from = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    byte* from_orig = dptr_storage::get_origin(from);

    if (!from_orig) {
        return;
    }

    traceable_object_meta* meta = managed_object::get_meta(from_orig);
    if (meta->is_forwarded()) {
        byte* to = managed_object::get_object(meta->forward_pointer());

        logging::debug() << "fix ptr: from " << (void*) from << " to " << (void*) to;

        if (dptr_storage::is_derived(from)) {
            dptr_storage::forward_derived_ptr(from, to);
        } else {
            gc_handle_access::set<std::memory_order_relaxed>(*handle, to);
        }
    }
}

void forwarding::move_cell(byte* from, byte* to, size_t size)
{
    assert(from);
    assert(to);
    memcpy(to, from, size);
}

}}}
