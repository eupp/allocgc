#ifndef DIPLOMA_CORE_ALLOCATOR_HPP
#define DIPLOMA_CORE_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/sys_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_allocator.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

class core_allocator
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;
    typedef boost::iterator_range<byte*> memory_range_type;

    core_allocator() = default;
    core_allocator(const core_allocator&) = default;
    core_allocator(core_allocator&&) = default;

    core_allocator& operator=(const core_allocator&) = default;
    core_allocator& operator=(core_allocator&&) = default;

    static byte* allocate(size_t size);

    static void deallocate(byte* ptr, size_t size);

    static size_t shrink();

    static memory_range_type memory_range();
private:
    typedef freelist_allocator<sys_allocator, varsize_policy> freelist_t;
    
    static freelist_t freelist;
};

}}}

#endif //DIPLOMA_CORE_ALLOCATOR_HPP
