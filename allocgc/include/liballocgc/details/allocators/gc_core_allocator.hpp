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
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/logging.hpp>
#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/gc_interface.hpp>

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

    gc_core_allocator();

    explicit gc_core_allocator(gc_launcher* gc);

    gc_core_allocator(const gc_core_allocator&) = default;
    gc_core_allocator(gc_core_allocator&&) = default;

    gc_core_allocator& operator=(const gc_core_allocator&) = default;
    gc_core_allocator& operator=(gc_core_allocator&&) = default;

    byte* allocate(size_t size);

    void deallocate(byte* ptr, size_t size);

    size_t shrink();

    memory_range_type memory_range();

    void set_heap_limit(size_t limit);
    void expand_heap();

    gc_run_stat gc(const gc_options& options);
private:
    typedef freelist_allocator<sys_allocator> freelist_alloc_t;
    typedef freelist_allocator<sys_allocator> fixsize_page_alloc_t;
    typedef bucket_allocator<fixsize_page_alloc_t, page_bucket_policy> bucket_alloc_t;

    typedef std::mutex mutex_t;

    static const size_t HEAP_START_LIMIT;

    static const double INCREASE_FACTOR;
    static const double MARK_THRESHOLD;
    static const double COLLECT_THRESHOLD;

    bool increase_heap_size(size_t size, std::unique_lock<mutex_t>* lock);
    void decrease_heap_size(size_t size);

    gc_launcher* m_gc_launcher;
    size_t m_heap_size;
    size_t m_heap_limit;
    size_t m_heap_maxlimit;
    bucket_alloc_t m_bucket_alloc;
    freelist_alloc_t m_freelist;
    mutex_t m_mutex;
};

}}}

#endif //ALLOCGC_GC_CORE_ALLOCATOR_HPP
