#ifndef ALLOCGC_DEFAULT_ALLOCATOR_HPP
#define ALLOCGC_DEFAULT_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details { namespace allocators {

class default_allocator
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;
    typedef boost::iterator_range<byte*> memory_range_type;

    default_allocator() = default;
    default_allocator(const default_allocator&) = default;
    default_allocator(default_allocator&&) = default;

    static byte* allocate(size_t size, size_t alignment = 0)
    {
        mem_allocated += size;
        return reinterpret_cast<byte*>(malloc(size));
    }

    static void deallocate(byte* ptr, size_t size, size_t alignment = 0)
    {
        mem_allocated -= size;
        free(ptr);
    }

    static size_t shrink()
    {
        return 0;
    }

    static size_t memory_allocated()
    {
        return mem_allocated;
    }

    static memory_range_type memory_range()
    {
        return memory_range_type(nullptr, nullptr);
    }

    friend bool operator==(const default_allocator& a, const default_allocator& b)
    {
        return true;
    }

    friend bool operator!=(const default_allocator& a, const default_allocator& b)
    {
        return false;
    }
private:
    // rough approximation of memory allocated to the needs of gc outside from the gc-heap.
    static size_t mem_allocated;
};

}}}

#endif //ALLOCGC_DEFAULT_ALLOCATOR_HPP
