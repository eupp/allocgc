#ifndef DIPLOMA_MSO_ALLOCATOR_HPP
#define DIPLOMA_MSO_ALLOCATOR_HPP

#include <cstring>
#include <array>

#include <libprecisegc/details/allocators/mpool_allocator.hpp>
#include <libprecisegc/details/allocators/pow2_bucket_policy.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/stl_adapter.hpp>

#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

class mso_allocator : private utils::noncopyable, private utils::nonmovable
{
    typedef pow2_bucket_policy<MIN_CELL_SIZE_BITS_CNT, LARGE_CELL_SIZE_BITS_CNT> bucket_policy;
public:
    typedef gc_alloc_response pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    mso_allocator();
    ~mso_allocator();

    gc_alloc_response allocate(const gc_alloc_request& rqst);

    gc_heap_stat collect(compacting::forwarding& frwd);
    void fix(const compacting::forwarding& frwd);
private:
    static const size_t BUCKET_COUNT = bucket_policy::BUCKET_COUNT;

    std::array<mpool_allocator, BUCKET_COUNT> m_allocs;
};

}}}

#endif //DIPLOMA_MSO_ALLOCATOR_HPP
