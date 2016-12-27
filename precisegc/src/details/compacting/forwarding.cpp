#include <libprecisegc/details/compacting/forwarding.hpp>

#include <cassert>

#include <libprecisegc/details/collectors/dptr_storage.hpp>
#include <libprecisegc/details/allocators/gc_box.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace compacting {

void forwarding::create(byte* from, byte* to)
{
    using namespace allocators;
    using namespace collectors;

    logging::debug() << "create forwarding: from " << (void*) from << " to " << (void*) to;

    gc_box::set_forward_pointer(from, to);
}

void forwarding::forward(gc_word* handle) const
{
    using namespace allocators;
    using namespace collectors;

    byte* from = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    byte* from_orig = dptr_storage::get_origin(from);

    if (!from_orig || !gc_box::is_forwarded(from_orig)) {
        return;
    }

    byte* to = gc_box::forward_pointer(from_orig);

    if (dptr_storage::is_derived(from)) {
        dptr_storage::forward_derived_ptr(from, to);
    } else {
        logging::debug() << "fix ptr: from " << (void*) from << " to " << (void*) to;

        gc_handle_access::set<std::memory_order_relaxed>(*handle, to);
    }
}

}}}
