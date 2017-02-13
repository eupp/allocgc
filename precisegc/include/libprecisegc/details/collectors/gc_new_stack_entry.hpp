#ifndef DIPLOMA_GC_NEW_STACK_ENTRY_HPP
#define DIPLOMA_GC_NEW_STACK_ENTRY_HPP

#include <cstddef>
#include <vector>
#include <memory>

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/gc_memory_descriptor.hpp>

namespace precisegc { namespace details { namespace collectors {

struct gc_new_stack_entry
{
public:
    typedef std::vector<size_t> offsets_storage_t;

    byte*                   obj_start;
    gc_memory_descriptor*   descriptor;
    offsets_storage_t       offsets;
    gc_new_stack_entry*     prev;
    bool                    meta_requested;
};

static_assert(sizeof(gc_new_stack_entry) <= gc_buf::size(), "gc_new_stack_entry is too large");

}}}

#endif //DIPLOMA_GC_NEW_STACK_ENTRY_HPP
