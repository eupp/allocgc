#ifndef DIPLOMA_DEBUG_LAYER_H
#define DIPLOMA_DEBUG_LAYER_H

#include <cstddef>

#include "types.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Alloc>
class debug_layer
{
public:

    debug_layer()
        : m_allocated_mem(0)
    {}

    byte* allocate(size_t size)
    {
        m_allocated_mem += size;
        return m_allocator.allocate(size);
    }

    void deallocate(byte* ptr, size_t size)
    {
        m_allocated_mem -= size;
        m_allocator.deallocate(ptr, size);
    }

    size_t get_allocated_mem_size() const
    {
        return m_allocated_mem;
    }

private:
    Alloc m_allocator;
    size_t m_allocated_mem;
};

}}}

#endif //DIPLOMA_DEBUG_LAYER_H
