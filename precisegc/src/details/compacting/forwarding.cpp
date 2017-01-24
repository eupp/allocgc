#include <libprecisegc/details/compacting/forwarding.hpp>

#include <cassert>

#include <libprecisegc/details/collectors/gc_tagging.hpp>
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

void forwarding::forward(gc_handle* handle) const
{
    using namespace allocators;
    using namespace collectors;

    byte* from = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    byte* from_obj_start  = gc_tagging::get_obj_start(from);
    if (!from_obj_start) {
        return;
    }

    byte* from_cell_start = gc_box::get_cell_start(from_obj_start);
    if (!gc_box::is_forwarded(from_cell_start)) {
        return;
    }

    byte* to_obj_start = gc_box::forward_pointer(from_cell_start);
    byte* to = to_obj_start;

    if (gc_tagging::is_derived(from)) {
        dptr_descriptor* dscr = gc_tagging::get_dptr_descriptor(from);

        from = dscr->m_derived;
        to  += dscr->m_derived - dscr->m_origin;

        dscr->m_origin  = to_obj_start;
        dscr->m_derived = to;
    } else {
        gc_handle_access::set<std::memory_order_relaxed>(*handle, to_obj_start);
    }

    logging::debug() << "fix ptr: from " << (void*) from << " to " << (void*) to;
}

}}}
