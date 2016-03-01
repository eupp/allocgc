#ifndef DIPLOMA_POOL_H
#define DIPLOMA_POOL_H

#include <cstddef>
#include <array>

#include "fixed_size_allocator.h"
#include "types.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename Alloc, typename InternalAlloc, typename BucketPolicy>
class bucket_allocator : private ebo<BucketPolicy>, private noncopyable
{
    static const size_t BUCKET_COUNT = BucketPolicy::BUCKET_COUNT;

    typedef typename Chunk::pointer_type pointer_type;
    typedef fixed_size_allocator<Chunk, Alloc, InternalAlloc> fixed_size_allocator_t;
    typedef std::array<fixed_size_allocator_t, BUCKET_COUNT> array_t;
public:

    bucket_allocator() = default;
    bucket_allocator(bucket_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        auto& bp = this->template get_base<BucketPolicy>();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.align(size);
        return m_buckets[ind].allocate(aligned_size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        auto& bp = this->template get_base<BucketPolicy>();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.align(size);
        m_buckets[ind].deallocate(ptr, aligned_size);
    }

private:
    array_t m_buckets;
};

}}}

#endif //DIPLOMA_POOL_H
