#ifndef DIPLOMA_FIXED_BLOCK_CACHE_HPP
#define DIPLOMA_FIXED_BLOCK_CACHE_HPP

#include <cstddef>
#include <cassert>
#include <array>
#include <stack>
#include <algorithm>
#include <mutex>

#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Alloc, size_t BlockSize, size_t Size>
class fixed_block_cache : private ebo<Alloc>, private noncopyable, private nonmovable
{
    static const size_t CacheSize = Size / BlockSize;
    typedef std::vector<byte*> cache_t;
    typedef std::mutex mutex_type;
public:
    typedef byte* pointer_type;

    fixed_block_cache()
    {
        m_cache.reserve(CacheSize);
    }

    ~fixed_block_cache()
    {
        for (auto p : m_cache) {
            get_allocator().deallocate(p, BlockSize);
        }
    }

    pointer_type allocate(size_t size)
    {
        if (size == BlockSize && !m_cache.empty()) {
            std::lock_guard<mutex_type> lock(m_mutex);
            byte* p = m_cache.back();
            m_cache.pop_back();
            return p;
        }
        return get_allocator().allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        if (size == BlockSize && m_cache.size() < CacheSize) {
            std::lock_guard<mutex_type> lock(m_mutex);
            m_cache.push_back(ptr);
            return;
        }
        get_allocator().deallocate(ptr, size);
    }

    const Alloc& get_allocator() const
    {
        return this->template get_base<Alloc>();
    }

    const Alloc& get_const_allocator() const
    {
        return this->template get_base<Alloc>();
    }

private:
    Alloc& get_allocator()
    {
        return this->template get_base<Alloc>();
    }

    cache_t m_cache;
    mutex_type m_mutex;
};

}}}

#endif //DIPLOMA_FIXED_BLOCK_CACHE_HPP
