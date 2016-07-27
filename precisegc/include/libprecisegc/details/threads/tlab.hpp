#ifndef DIPLOMA_TLAB_HPP
#define DIPLOMA_TLAB_HPP

#include <cassert>

#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/page_allocator.hpp>
#include <libprecisegc/details/allocators/bucket_allocator.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/allocators/pow2_bucket_policy.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_compact.h>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details { namespace threads {

class tlab : public utils::noncopyable, public utils::nonmovable
{
    typedef allocators::bucket_allocator<
            allocators::managed_pool_chunk,
            allocators::page_allocator,
            allocators::default_allocator,
            allocators::pow2_bucket_policy<MIN_CELL_SIZE_BITS_CNT, LARGE_CELL_SIZE_BITS_CNT>,
            utils::dummy_mutex
    > alloc_t;
public:
    tlab() = default;

    managed_ptr allocate(size_t size)
    {
        assert(size <= LARGE_CELL_SIZE);
        return m_alloc.allocate(size);
    }

    template <typename Forwarding>
    size_t sweep(gc_compacting compacting, Forwarding& frwd)
    {
        size_t freed = m_alloc.shrink();
        if (compacting == gc_compacting::ENABLED) {
            auto& bp = m_alloc.get_bucket_policy();
            for (size_t i = 0; i < bp.BUCKET_COUNT; ++i) {
                auto rng = m_alloc.memory_range(i);
                two_finger_compact(rng, bp.bucket_size(i), frwd);
            }
        }
        return freed;
    }

    template <typename Forwarding>
    void fix_pointers(const Forwarding& frwd)
    {
        auto& bp = m_alloc.get_bucket_policy();
        for (size_t i = 0; i < bp.BUCKET_COUNT; ++i) {
            auto rng = m_alloc.memory_range(i);
            ::precisegc::details::fix_pointers(rng.begin(), rng.end(), bp.bucket_size(i), frwd);
        }
    }

    void unmark()
    {
        m_alloc.apply_to_chunks([] (allocators::managed_pool_chunk& chunk) {
            chunk.unmark();
        });
    }
private:
    alloc_t m_alloc;
};

}}}

#endif //DIPLOMA_TLAB_HPP
