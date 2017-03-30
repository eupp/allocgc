#ifndef ALLOCGC_NULL_ALLOCATOR_HPP
#define ALLOCGC_NULL_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/details/types.hpp>

namespace allocgc { namespace details { namespace allocators {

class null_allocator
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;
    typedef boost::iterator_range<byte*> memory_range_type;

    null_allocator() = default;
    null_allocator(const null_allocator&) = default;
    null_allocator(null_allocator&&) = default;

    null_allocator& operator=(const null_allocator&) = default;
    null_allocator& operator=(null_allocator&&) = default;

    byte* allocate(size_t size)
    {
        return nullptr;
    }

    void deallocate(byte* ptr, size_t size)
    {
        return;
    }

    size_t shrink()
    {
        return 0;
    }

    memory_range_type memory_range()
    {
        return memory_range_type(nullptr, nullptr);
    }

    bool empty() const
    {
        return true;
    }

    friend bool operator==(const null_allocator& a, const null_allocator& b)
    {
        return true;
    }

    friend bool operator!=(const null_allocator& a, const null_allocator& b)
    {
        return false;
    }
};

}}}

#endif //ALLOCGC_NULL_ALLOCATOR_HPP
