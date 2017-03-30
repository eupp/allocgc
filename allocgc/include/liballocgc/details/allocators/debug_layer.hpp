#ifndef ALLOCGC_DEBUG_LAYER_H
#define ALLOCGC_DEBUG_LAYER_H

#include <cstddef>

#include "liballocgc/gc_common.hpp"

namespace allocgc { namespace details { namespace allocators {

template <typename UpstreamAlloc>
class debug_layer
{
    static_assert(std::is_same<typename UpstreamAlloc::pointer_type, byte*>::value,
                  "debug_layer should be used with raw memory allocator");
public:
    typedef byte* pointer_type;
    typedef typename UpstreamAlloc::alloc_tag alloc_tag;
    typedef typename UpstreamAlloc::memory_range_type memory_range_type;

    debug_layer()
        : m_allocated_mem(0)
    {}

    debug_layer(const debug_layer&) = default;
    debug_layer(debug_layer&&) = default;

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

    size_t shrink()
    {
        return m_allocator.shrink();
    }

    memory_range_type memory_range()
    {
        return m_allocator.memory_range();
    }

    size_t get_allocated_mem_size() const
    {
        return m_allocated_mem;
    }
private:
    UpstreamAlloc m_allocator;
    size_t m_allocated_mem;
};

}}}

#endif //ALLOCGC_DEBUG_LAYER_H
