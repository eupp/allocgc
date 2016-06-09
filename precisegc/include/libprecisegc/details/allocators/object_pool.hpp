#ifndef DIPLOMA_OBJECT_POOL_HPP
#define DIPLOMA_OBJECT_POOL_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

class object_pool
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;

    object_pool() = default;
    object_pool(const object_pool&) = default;
    object_pool(object_pool&&) = default;

    byte* allocate(size_t size)
    {
        return pool.allocate(size);
    }

    void deallocate(byte* ptr, size_t size)
    {
        pool.deallocate(ptr, size);
    }
private:
    typedef allocators::intrusive_list_allocator<
            allocators::freelist_pool_chunk, allocators::default_allocator
    > object_pool_t;

    static thread_local object_pool_t pool;
};

}}}

#endif //DIPLOMA_OBJECT_POOL_HPP
