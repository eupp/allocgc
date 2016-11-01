#ifndef DIPLOMA_CORE_ALLOCATOR_HPP
#define DIPLOMA_CORE_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <mutex>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/sys_allocator.hpp>
#include <libprecisegc/details/allocators/bucket_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_allocator.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

class core_allocator
{
    class page_bucket_policy
    {
    public:
        static const size_t BUCKET_COUNT = LARGE_CELL_SIZE_BITS_CNT - MIN_CELL_SIZE_BITS_CNT  + 1;

        static size_t bucket(size_t size);
        static size_t bucket_size(size_t i);
    };
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

    static void expand_heap();

    static size_t shrink();

    static memory_range_type memory_range();
private:
    typedef freelist_allocator<sys_allocator, varsize_policy> freelist_alloc_t;
    typedef freelist_allocator<sys_allocator, fixsize_policy> fixsize_page_alloc_t;
    typedef bucket_allocator<fixsize_page_alloc_t, page_bucket_policy> bucket_alloc_t;

    typedef std::mutex mutex_t;

    static const size_t MAX_BUCKETIZE_SIZE = MANAGED_CHUNK_OBJECTS_COUNT * LARGE_CELL_SIZE;

    static const double INCREASE_FACTOR;
    static const double MARK_THRESHOLD;
    static const double COLLECT_THRESHOLD;

    static bool check_heap_size(size_t alloc_size);

    static bucket_alloc_t bucket_alloc;
    static freelist_alloc_t freelist;
    static mutex_t mutex;

    static size_t heap_size;
    static size_t cur_heap_size;
    static size_t max_heap_size;
};

}}}

#endif //DIPLOMA_CORE_ALLOCATOR_HPP
