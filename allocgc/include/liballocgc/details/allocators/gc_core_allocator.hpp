#ifndef ALLOCGC_GC_CORE_ALLOCATOR_HPP
#define ALLOCGC_GC_CORE_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <atomic>
#include <mutex>

#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/details/allocators/sys_allocator.hpp>
#include <liballocgc/details/allocators/bucket_allocator.hpp>
#include <liballocgc/details/allocators/freelist_allocator.hpp>
#include <liballocgc/details/gc_hooks.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/logging.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_core_allocator
{
    static const size_t MAX_BUCKETIZE_SIZE = MANAGED_CHUNK_OBJECTS_COUNT * LARGE_CELL_SIZE;

    class page_bucket_policy
    {
    public:
        static const size_t BUCKET_COUNT = MAX_BUCKETIZE_SIZE / LARGE_CELL_SIZE;

        static size_t bucket(size_t size);
        static size_t bucket_size(size_t i);
    };
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;
    typedef boost::iterator_range<byte*> memory_range_type;

    gc_core_allocator() = default;
    gc_core_allocator(const gc_core_allocator&) = default;
    gc_core_allocator(gc_core_allocator&&) = default;

    gc_core_allocator& operator=(const gc_core_allocator&) = default;
    gc_core_allocator& operator=(gc_core_allocator&&) = default;

    static byte* allocate(size_t size, bool zeroing = true);

    static void deallocate(byte* ptr, size_t size);

    static size_t shrink();

    static memory_range_type memory_range();
private:
    typedef freelist_allocator<sys_allocator> freelist_alloc_t;
    typedef freelist_allocator<sys_allocator> fixsize_page_alloc_t;
    typedef bucket_allocator<fixsize_page_alloc_t, page_bucket_policy> bucket_alloc_t;

    typedef std::mutex mutex_t;

    static bucket_alloc_t bucket_alloc;
    static freelist_alloc_t freelist;
    static mutex_t mutex;
};

}}}

#endif //ALLOCGC_GC_CORE_ALLOCATOR_HPP
