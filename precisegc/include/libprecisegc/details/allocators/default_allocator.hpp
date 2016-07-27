#ifndef DIPLOMA_DEFAULT_ALLOCATOR_HPP
#define DIPLOMA_DEFAULT_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

class default_allocator
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;

    default_allocator() = default;
    default_allocator(const default_allocator&) = default;
    default_allocator(default_allocator&&) = default;

    byte* allocate(size_t size)
    {
        return reinterpret_cast<byte*>(malloc(size));
    }

    void deallocate(byte* ptr, size_t size)
    {
        free(ptr);
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
