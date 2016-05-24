#ifndef DIPLOMA_POOL_H
#define DIPLOMA_POOL_H

#include <cstddef>
#include <array>
#include <utility>
#include <mutex>

#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/utils/lock_range.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>


namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename Alloc, typename InternalAlloc, typename BucketPolicy, typename Lock>
class bucket_allocator : private ebo<BucketPolicy>, private noncopyable
{
    static const size_t BUCKET_COUNT = BucketPolicy::BUCKET_COUNT;

    typedef list_allocator<Chunk, Alloc, InternalAlloc> fixed_size_allocator_t;
    typedef std::array<fixed_size_allocator_t, BUCKET_COUNT> array_t;
    typedef std::array<Lock, BUCKET_COUNT> lock_array_t;
public:
    typedef typename Chunk::pointer_type pointer_type;
    typedef utils::locked_range<typename fixed_size_allocator_t::memory_range_type, Lock> range_type;

    bucket_allocator() = default;
    bucket_allocator(bucket_allocator&&) = default;

    std::pair<pointer_type, size_t> allocate(size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        std::lock_guard<Lock> lock(m_locks[ind]);
        pointer_type p = m_buckets[ind].allocate(aligned_size);
        return std::make_pair(p, aligned_size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        std::lock_guard<Lock> lock(m_locks[ind]);
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

    template <typename Functor>
    void apply_to_chunks(Functor& f)
    {
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            std::lock_guard<Lock> lock(m_locks[i]);
            m_buckets[i].apply_to_chunks(f);
        }
    }

    template <typename Functor>
    void apply_to_chunks(const Functor& f)
    {
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            std::lock_guard<Lock> lock(m_locks[i]);
            m_buckets[i].apply_to_chunks(f);
        }
    }

    range_type memory_range(size_t bucket_ind)
    {
        return utils::lock_range(m_buckets[bucket_ind].memory_range(), m_locks[bucket_ind]);
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
