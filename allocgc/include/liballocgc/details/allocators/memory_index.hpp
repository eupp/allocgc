#ifndef ALLOCGC_MEMORY_INDEX_HPP
#define ALLOCGC_MEMORY_INDEX_HPP

#include <liballocgc/details/allocators/index_tree.hpp>
#include <liballocgc/details/allocators/memory_descriptor.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>
#include <liballocgc/details/gc_cell.hpp>

namespace allocgc { namespace details { namespace allocators {

class memory_index
{
public:
    static inline void init()
    {
        indexer.init();
    }

    static inline void clear()
    {
        indexer.clear();
    }

    static inline void index_stack_memory(const byte* mem, size_t size, byte* descriptor)
    {
        indexer.index(mem, size, memory_descriptor::make_stack_descriptor(descriptor));
    }

    static inline void index_gc_heap_memory(const byte* mem, size_t size, gc_memory_descriptor* descriptor)
    {
        indexer.index(mem, size, memory_descriptor::make_gc_heap_descriptor(descriptor));
    }

    static inline void deindex(const byte* mem, size_t size)
    {
        indexer.deindex(mem, size);
    }

    static inline memory_descriptor get_descriptor(const byte* mem)
    {
        return indexer.get_descriptor(mem);
    }

    static inline gc_cell get_gc_cell(byte* ptr)
    {
        return gc_cell::from_internal_ptr(ptr, get_descriptor(ptr).to_gc_descriptor());
    }

    static inline size_t size()
    {
        return indexer.size();
    }
private:
    static index_tree indexer;
};

}}}

#endif //ALLOCGC_MEMORY_INDEX_HPP