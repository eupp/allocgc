#ifndef DIPLOMA_POOL_H
#define DIPLOMA_POOL_H

#include <cstddef>
#include <array>

#include "fixed_size_allocator.h"
#include "types.h"
#include "../util.h"
#include "../mutex.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename Alloc, typename InternalAlloc, typename BucketPolicy, typename Lock>
class bucket_allocator : private ebo<BucketPolicy>, private noncopyable
{
    static const size_t BUCKET_COUNT = BucketPolicy::BUCKET_COUNT;

    typedef typename Chunk::pointer_type pointer_type;
    typedef fixed_size_allocator<Chunk, Alloc, InternalAlloc> fixed_size_allocator_t;
    typedef std::array<fixed_size_allocator_t, BUCKET_COUNT> array_t;
    typedef std::array<Lock, BUCKET_COUNT> lock_array_t;
public:

    bucket_allocator() = default;
    bucket_allocator(bucket_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        auto& bp = this->template get_base<BucketPolicy>();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        lock_guard<Lock> lock(m_locks[ind]);
        return m_buckets[ind].allocate(aligned_size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        auto& bp = this->template get_base<BucketPolicy>();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(size);
        lock_guard<Lock> lock(m_locks[ind]);
        m_buckets[ind].deallocate(ptr, aligned_size);
    }

private:
    array_t m_buckets;
    lock_array_t m_locks;
};

}}}

#endif //DIPLOMA_POOL_H
