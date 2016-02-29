#ifndef DIPLOMA_FIXED_SIZE_ALLOCATOR_H
#define DIPLOMA_FIXED_SIZE_ALLOCATOR_H

#include <cstddef>
#include <vector>

namespace precisegc { namespace details { namespace allocators {

template <typename chunk, typename Alloc, typename InnerAlloc>
class fixed_size_allocator
{
public:
    typedef typename chunk::pointer_type pointer_type;

    pointer_type allocate(size_t obj_size);
    void deallocate(pointer_type ptr, size_t obj_size);

private:
    std::vector<chunk, stl_adapter<InnerAlloc>> m_chunks;
    size_t m_alloc_chunk;
    Alloc m_allocator;
};

}}}

#endif //DIPLOMA_FIXED_SIZE_ALLOCATOR_H
