#ifndef DIPLOMA_BUCKET_ALLOCATOR_HPP
#define DIPLOMA_BUCKET_ALLOCATOR_HPP

#include <cstddef>
#include <array>
#include <utility>
#include <mutex>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename Alloc, typename BucketPolicy>
class bucket_allocator : private utils::noncopyable
{
    static const size_t BUCKET_COUNT = BucketPolicy::BUCKET_COUNT;

    typedef std::array<Alloc, BUCKET_COUNT> array_t;
public:
    typedef typename Alloc::pointer_type pointer_type;
//    typedef typename Alloc::memory_range_type memory_range_type;

    bucket_allocator() = default;
    bucket_allocator(bucket_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        size_t ind = BucketPolicy::bucket(size);
        size_t aligned_size = BucketPolicy::bucket_size(ind);
        assert(ind < BucketPolicy::BUCKET_COUNT);
        return m_buckets[ind].allocate(aligned_size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        size_t ind = BucketPolicy::bucket(size);
        size_t aligned_size = BucketPolicy::bucket_size(ind);
        assert(ind < BucketPolicy::BUCKET_COUNT);
        m_buckets[ind].deallocate(ptr, aligned_size);
    }

    void deallocate(pointer_type ptr, size_t bucket_ind, size_t bucket_size)
    {
        assert(bucket_ind < BucketPolicy::BUCKET_COUNT);
        m_buckets[bucket_ind].deallocate(ptr, bucket_size);
    }

//    size_t shrink()
//    {
//        size_t shrunk = 0;
//        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
//            shrunk += m_buckets[i].shrink();
//        }
//        return shrunk;
//    }
//
//    bool empty() const
//    {
//        bool flag = true;
//        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
//            flag &= m_buckets[i].empty();
//        }
//        return flag;
//    }

//    template <typename Functor>
//    void apply(Functor&& f)
//    {
//        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
//            f(m_buckets[i]);
//        }
//    }
//
//    template <typename Functor>
//    void apply(size_t bucket_ind, Functor&& f)
//    {
//        f(m_buckets[bucket_ind]);
//    }
//
//    memory_range_type memory_range(size_t bucket_ind)
//    {
//        return m_buckets[bucket_ind].memory_range();
//    }

    Alloc& get_bucket_alloc(size_t bucket_ind)
    {
        return m_buckets[bucket_ind];
    }
private:
    array_t m_buckets;
};

}}}

#endif //DIPLOMA_BUCKET_ALLOCATOR_HPP
