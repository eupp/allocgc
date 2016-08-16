#ifndef DIPLOMA_PAGE_ALLOCATOR_HPP
#define DIPLOMA_PAGE_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

/**
 * page_allocator - allocates memory regions which size is multiple of PAGE_SIZE, aligned by its size
 */
class page_allocator
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;

    page_allocator() = default;
    page_allocator(const page_allocator&) = default;
    page_allocator(page_allocator&&) = default;

    byte* allocate(size_t size, size_t alignment = 0)
    {
        assert(size != 0 /*&& size % PAGE_SIZE == 0*/);
        gc_initiation_point(initiation_point_type::HEAP_EXPANSION,
                            initiation_point_data::create_heap_expansion_data(size));
        byte* page = reinterpret_cast<byte*>(aligned_alloc(alignment, size));
        gc_register_page(page, size);
        return page;
    }

    void deallocate(byte* ptr, size_t size, size_t alignment = 0)
    {
//        assert(size != 0 && size % PAGE_SIZE == 0);
        gc_deregister_page(ptr, size);
        free(ptr);
    }
};

}}}

#endif //DIPLOMA_PAGE_ALLOCATOR_HPP
