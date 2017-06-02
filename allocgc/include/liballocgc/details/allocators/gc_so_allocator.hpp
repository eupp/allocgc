#ifndef ALLOCGC_GC_SO_ALLOCATOR_HPP
#define ALLOCGC_GC_SO_ALLOCATOR_HPP

#include <cstring>
#include <array>
#include <utility>

#include <liballocgc/gc_common.hpp>

#include <liballocgc/details/allocators/gc_pool_allocator.hpp>
#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/details/allocators/stl_adapter.hpp>

#include <liballocgc/details/utils/static_thread_pool.hpp>
#include <liballocgc/details/utils/dynarray.hpp>
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

    explicit gc_so_allocator(gc_core_allocator* core_alloc);

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    gc_heap_stat collect(compacting::forwarding& frwd, thread_pool_t& thread_pool);
    void fix(const compacting::forwarding& frwd, thread_pool_t& thread_pool);
    void finalize();
private:
    // we have buckets for each 2^k size
    // i.g. [32, 64, 128, 256, ...]
    static const size_t BUCKET_COUNT = LARGE_CELL_SIZE_LOG2 - MIN_CELL_SIZE_LOG2 + 1;
    static const size_t MAX_SIZE = LARGE_CELL_SIZE;

    static size_t SZ_CLS[BUCKET_COUNT];

    static_assert(BUCKET_COUNT <= std::numeric_limits<byte>::max(), "Too many buckets");

    std::array<byte, MAX_SIZE> m_sztbl;
    std::array<gc_pool_allocator, BUCKET_COUNT> m_buckets;
};

}}}

#endif //ALLOCGC_GC_SO_ALLOCATOR_HPP
