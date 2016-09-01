#ifndef DIPLOMA_POOL_H
#define DIPLOMA_POOL_H

#include <cstddef>
#include <array>
#include <utility>
#include <mutex>

#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename Alloc, typename InternalAlloc, typename BucketPolicy, typename Lock>
class bucket_allocator : private utils::ebo<BucketPolicy>, private utils::noncopyable
{
    static const size_t BUCKET_COUNT = BucketPolicy::BUCKET_COUNT;

    typedef list_allocator<Chunk, Alloc, InternalAlloc, Lock> fixed_size_allocator_t;
    typedef std::array<fixed_size_allocator_t, BUCKET_COUNT> array_t;
    typedef std::array<Lock, BUCKET_COUNT> lock_array_t;
public:
    typedef utils::block_ptr<typename Chunk::pointer_type> pointer_type;
    typedef typename fixed_size_allocator_t::memory_range_type memory_range_type;

    bucket_allocator() = default;
    bucket_allocator(bucket_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        return utils::block_ptr(m_buckets[ind].allocate(aligned_size), aligned_size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = ptr.size() != 0 ? ptr.size() : bp.bucket_size(ind);
        m_buckets[ind].deallocate(ptr, aligned_size);
    }

    size_t shrink()
    {
        size_t shrunk = 0;
        auto& bp = get_bucket_policy();
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            shrunk += m_buckets[i].shrink(bp.bucket_size(i));
        }
        return shrunk;
    }

    bool empty() const
    {
        bool flag = true;
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            flag &= m_buckets[i].empty();
        }
        return flag;
    }

    template <typename Functor>
    void apply_to_chunks(Functor&& f)
    {
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            m_buckets[i].apply_to_chunks(f);
        }
    }

    memory_range_type memory_range(size_t bucket_ind)
    {
        return m_buckets[bucket_ind].memory_range();
    }

    BucketPolicy& get_bucket_policy()
    {
        return this->template get_base<BucketPolicy>();
    }
private:
    array_t m_buckets;
};

}}}

#endif //DIPLOMA_POOL_H
