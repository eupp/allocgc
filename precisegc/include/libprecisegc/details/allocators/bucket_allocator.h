#ifndef DIPLOMA_POOL_H
#define DIPLOMA_POOL_H

#include <cstddef>
#include <array>
#include <utility>

#include "fixed_size_allocator.h"
#include "types.h"
#include "../util.h"
#include "../mutex.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename Alloc, typename InternalAlloc, typename BucketPolicy, typename Lock>
class bucket_allocator : private ebo<BucketPolicy>, private noncopyable
{
    static const size_t BUCKET_COUNT = BucketPolicy::BUCKET_COUNT;

    typedef fixed_size_allocator<Chunk, Alloc, InternalAlloc> fixed_size_allocator_t;
    typedef std::array<fixed_size_allocator_t, BUCKET_COUNT> array_t;
    typedef std::array<Lock, BUCKET_COUNT> lock_array_t;
public:
    typedef typename Chunk::pointer_type pointer_type;
    typedef typename fixed_size_allocator_t::range_type range_type;

    bucket_allocator() = default;
    bucket_allocator(bucket_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        lock_guard<Lock> lock(m_locks[ind]);
        return m_buckets[ind].allocate(aligned_size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        lock_guard<Lock> lock(m_locks[ind]);
        m_buckets[ind].deallocate(ptr, aligned_size);
    }

    void reset_cache()
    {
        for (auto& alloc: m_buckets) {
            alloc.reset_cache();
        }
    }

    range_type range(size_t bucket_ind)
    {
        return m_buckets[bucket_ind].range();
    }

    BucketPolicy& get_bucket_policy()
    {
        return this->template get_base<BucketPolicy>();
    }

private:
    array_t m_buckets;
    lock_array_t m_locks;
};

}}}

#endif //DIPLOMA_POOL_H
