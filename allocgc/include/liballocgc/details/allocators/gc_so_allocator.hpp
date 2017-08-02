#ifndef ALLOCGC_GC_SO_ALLOCATOR_HPP
#define ALLOCGC_GC_SO_ALLOCATOR_HPP

#include <cstring>
#include <array>
#include <utility>

#include <liballocgc/gc_common.hpp>

#include <liballocgc/details/allocators/gc_pool_allocator.hpp>
#include <liballocgc/details/allocators/gc_bucket_policy.hpp>
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

    gc_collect_stat collect(compacting::forwarding& frwd, thread_pool_t& thread_pool);
    void fix(const compacting::forwarding& frwd, thread_pool_t& thread_pool);
    void finalize();

    gc_memstat stats();
private:
    std::array<gc_pool_allocator, gc_bucket_policy::BUCKET_COUNT> m_buckets;
    gc_bucket_policy m_bucket_policy;
};

}}}

#endif //ALLOCGC_GC_SO_ALLOCATOR_HPP
