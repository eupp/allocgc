#ifndef ALLOCGC_GC_SO_ALLOCATOR_HPP
#define ALLOCGC_GC_SO_ALLOCATOR_HPP

#include <cstring>
#include <array>
#include <utility>

#include <liballocgc/details/allocators/gc_pool_allocator.hpp>
#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/details/allocators/stl_adapter.hpp>

#include <liballocgc/details/utils/static_thread_pool.hpp>
#include <liballocgc/details/utils/utility.hpp>

#include <liballocgc/details/compacting/forwarding.hpp>

#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_so_allocator : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef stateful_alloc_tag alloc_tag;
    typedef utils::static_thread_pool thread_pool_t;

    gc_so_allocator();

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    gc_heap_stat collect(compacting::forwarding& frwd, thread_pool_t& thread_pool);
    void fix(const compacting::forwarding& frwd, thread_pool_t& thread_pool);
    void finalize();
private:
    // we have buckets for each 2^k size
    // i.g. [32, 64, 128, 256, ...]
    static const size_t BUCKET_COUNT = LARGE_CELL_SIZE_LOG2 - MIN_CELL_SIZE_LOG2 + 1;

    typedef std::pair<size_t, gc_pool_allocator> bucket_t;

    std::array<bucket_t, BUCKET_COUNT> m_buckets;
};

}}}

#endif //ALLOCGC_GC_SO_ALLOCATOR_HPP
