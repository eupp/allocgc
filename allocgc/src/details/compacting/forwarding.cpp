#include <liballocgc/details/compacting/forwarding.hpp>

#include <cassert>

#include <liballocgc/details/allocators/gc_box.hpp>
#include <liballocgc/details/logging.hpp>
#include <liballocgc/details/allocators/gc_box_handle.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>

namespace allocgc { namespace details { namespace compacting {

void forwarding::create(byte* from, byte* to)
{
    using namespace allocators;

    logging::debug() << "create forwarding: from " << (void*) from << " to " << (void*) to;

    gc_box::set_forward_pointer(from, to);
}

void forwarding::forward(gc_handle* handle) const
{
    using namespace allocators;

    byte* from = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    if (!from) {
        return;
    }
    gc_box_handle from_cell       = allocators::memory_index::get_gc_cell(from);
    byte* from_cell_start   = from_cell.box_addr();
    byte* from_obj_start    = gc_box::get_obj_start(from_cell_start);

    if (!gc_box::is_forwarded(from_cell_start)) {
        return;
    }

    byte* to_obj_start = gc_box::forward_pointer(from_cell_start);
    byte* to = to_obj_start + (from - from_obj_start);

    gc_handle_access::set<std::memory_order_relaxed>(*handle, to);

    logging::debug() << "fix ptr: from " << (void*) from << " to " << (void*) to;
}

}}}
