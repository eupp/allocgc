#ifndef ALLOCGC_GC_NEW_STACK_ENTRY_HPP
#define ALLOCGC_GC_NEW_STACK_ENTRY_HPP

#include <cstddef>
#include <vector>
#include <memory>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>

namespace allocgc { namespace details { namespace collectors {

struct gc_new_stack_entry
{
public:
    typedef std::vector<size_t> offsets_storage_t;
    typedef allocators::gc_memory_descriptor gc_memory_descriptor;

    byte*                   obj_start;
    size_t                  obj_size;
    gc_memory_descriptor*   descriptor;
    offsets_storage_t       offsets;
    gc_new_stack_entry*     prev;
    bool                    meta_requested;
    bool                    zeroing_flag;
};

static_assert(sizeof(gc_new_stack_entry) <= gc_buf::size(), "gc_new_stack_entry is too large");

}}}

#endif //ALLOCGC_GC_NEW_STACK_ENTRY_HPP
