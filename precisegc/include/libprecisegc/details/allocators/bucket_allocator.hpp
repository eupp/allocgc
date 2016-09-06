#ifndef DIPLOMA_POOL_H
#define DIPLOMA_POOL_H

#include <cstddef>
#include <array>
#include <utility>
#include <mutex>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename Alloc, typename BucketPolicy>
class bucket_allocator : private utils::ebo<BucketPolicy>, private utils::noncopyable
{
    static const size_t BUCKET_COUNT = BucketPolicy::BUCKET_COUNT;

    typedef std::array<Alloc, BUCKET_COUNT> array_t;
public:
    typedef typename Alloc::pointer_type pointer_type;
    typedef typename Alloc::memory_range_type memory_range_type;

    bucket_allocator() = default;
    bucket_allocator(bucket_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        return m_buckets[ind].allocate(aligned_size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        auto& bp = get_bucket_policy();
        size_t ind = bp.bucket(size);
        size_t aligned_size = bp.bucket_size(ind);
        m_buckets[ind].deallocate(ptr, aligned_size);
    }

    size_t shrink()
    {
        size_t shrunk = 0;
        auto& bp = get_bucket_policy();
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            shrunk += m_buckets[i].shrink();
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
