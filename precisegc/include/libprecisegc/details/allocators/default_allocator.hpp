#ifndef DIPLOMA_DEFAULT_ALLOCATOR_HPP
#define DIPLOMA_DEFAULT_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc { namespace details { namespace allocators {

class default_allocator
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;
    typedef boost::iterator_range<byte*> memory_range_type;

    default_allocator() = default;
    default_allocator(const default_allocator&) = default;
    default_allocator(default_allocator&&) = default;

    byte* allocate(size_t size, size_t alignment = 0)
    {
        return reinterpret_cast<byte*>(malloc(size));
    }

    void deallocate(byte* ptr, size_t size, size_t alignment = 0)
    {
        free(ptr);
    }

    size_t shrink()
    {
        return 0;
    }

    memory_range_type memory_range()
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
};

}}}

#endif //DIPLOMA_DEFAULT_ALLOCATOR_HPP
