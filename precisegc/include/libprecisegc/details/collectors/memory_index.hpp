#ifndef DIPLOMA_MEMORY_INDEX_HPP
#define DIPLOMA_MEMORY_INDEX_HPP

#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/collectors/index_tree.hpp>

namespace precisegc { namespace details { namespace collectors {

class memory_index
{
public:
    static inline void add_to_index(byte* mem, size_t size, memory_descriptor* entry)
    {
        indexer.add_to_index(mem, size, entry);
    }

    static inline void remove_from_index(byte* mem, size_t size)
    {
        indexer.remove_from_index(mem, size);
    }

    static inline memory_descriptor* index(byte* mem)
    {
        return indexer.index(mem);
    }
private:
    typedef index_tree<memory_descriptor> index_tree_t;

    static index_tree_t indexer;
};

}}}

#endif //DIPLOMA_MEMORY_INDEX_HPP