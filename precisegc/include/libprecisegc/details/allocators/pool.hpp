#ifndef DIPLOMA_POOL_HPP
#define DIPLOMA_POOL_HPP

#include <mutex>

#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename T, typename Lock>
class pool
{
public:
    pool()
        : m_alloc_cnt(0)
        , m_dealloc_cnt(0)
    {}

    template <typename... Args>
    T* create(Args&&... args)
    {
        byte* ptr = allocate();
        new (ptr) T(std::forward<Args>(args)...);
        return reinterpret_cast<T*>(ptr);
    }

    void destroy(T* ptr)
    {
        ptr->~T();
        deallocate(reinterpret_cast<byte*>(ptr));
    }
private:
    typedef freelist_allocator<
              intrusive_list_allocator<
                      freelist_pool_chunk
                    , default_allocator
                    , single_chunk_with_search_cache
                    , utils::dummy_mutex
                >
            , fixsize_policy
        > alloc_t;

    static const size_t SHRINK_LOWER_BOUND = freelist_pool_chunk::DEFAULT_CHUNK_SIZE;

    byte* allocate()
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        ++m_alloc_cnt;
        return m_alloc.allocate(sizeof(T));
    }

    void deallocate(byte* ptr)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        m_alloc.deallocate(ptr, sizeof(T));
        ++m_dealloc_cnt;
        if (m_alloc_cnt > SHRINK_LOWER_BOUND && m_alloc_cnt <= 2 * m_dealloc_cnt) {
            m_alloc.shrink(m_alloc_size);
            m_alloc_cnt >>= 1;
            m_dealloc_cnt = 0;
        }
    }

    alloc_t m_alloc;
    size_t m_alloc_size;
    size_t m_alloc_cnt;
    size_t m_dealloc_cnt;
    Lock m_lock;
};

}}}

#endif //DIPLOMA_POOL_HPP
