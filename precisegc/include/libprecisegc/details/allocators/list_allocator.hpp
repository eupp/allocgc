#ifndef DIPLOMA_FIXED_SIZE_ALLOCATOR_H
#define DIPLOMA_FIXED_SIZE_ALLOCATOR_H

#include <cassert>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <memory>
#include <list>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/stl_adapter.hpp>
#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk,
          typename UpstreamAlloc,
          typename InternalAlloc,
          template <typename> class CachePolicy,
          typename Lock>
class list_allocator : private utils::ebo<UpstreamAlloc>, private utils::noncopyable, private utils::nonmovable
{
    typedef std::list<Chunk, stl_adapter<Chunk, InternalAlloc>> list_t;
    typedef typename list_t::iterator iterator_t;
    typedef typename UpstreamAlloc::pointer_type internal_pointer_type;
    typedef CachePolicy<iterator_t> cache_t;
public:
    typedef typename Chunk::pointer_type pointer_type;
    typedef stateful_alloc_tag alloc_tag;
    typedef utils::flattened_range<typename list_t::iterator> memory_range_type;
//    typedef utils::locked_range<
//                  utils::flattened_range<boost::iterator_range<typename list_t::iterator>>
//                , Lock
//            > memory_range_type;

    list_allocator()
    {
        m_cache.update(create_fake_chunk());
    }

    ~list_allocator()
    {
        for (auto it = m_chunks.begin(), end = std::prev(m_chunks.end()); it != end; ) {
            it = destroy_chunk(it);
        }
    }

    pointer_type allocate(size_t size)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        return allocate_unsafe(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        auto dealloc_chunk = std::find_if(m_chunks.begin(), m_chunks.end(),
                                          [&ptr] (const Chunk& chk) { return chk.contains(ptr); });
        if (dealloc_chunk != m_chunks.end()) {
            dealloc_chunk->deallocate(ptr, size);
            if (dealloc_chunk->empty()) {
                destroy_chunk(dealloc_chunk);
            }
        }
    }

    size_t shrink()
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        size_t shrunk = 0;
        for (auto it = m_chunks.begin(), end = std::prev(m_chunks.end()); it != end; ) {
            if (it->empty()) {
                shrunk += it->get_mem_size();
                it = destroy_chunk(it);
            } else {
                ++it;
            }
        }
        return shrunk;
    }

    bool empty() const
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        return m_chunks.empty();
    }

    void reset_cache()
    {
        m_cache.update(m_chunks.begin());
    }

    template <typename Functor>
    void apply_to_chunks(Functor&& f)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        std::for_each(m_chunks.begin(), std::prev(m_chunks.end()), std::forward<Functor>(f));
    }

    memory_range_type memory_range()
    {
//        std::unique_lock<Lock> lock_guard(m_lock);
//        return memory_range_type(utils::flatten_range(m_chunks), std::move(lock_guard));
        return utils::flatten_range(m_chunks.begin(), std::prev(m_chunks.end()));
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }
private:
    pointer_type allocate_unsafe(size_t size)
    {
        if (!m_cache.memory_available(m_chunks.begin(), m_chunks.end())) {
            auto new_chunk = create_chunk(size);
            auto first = m_chunks.begin();
            m_cache.update(first != std::prev(m_chunks.end()) && first->memory_available() ? first : new_chunk);
        }
        return m_cache.allocate(size);
    }

    typename list_t::iterator create_fake_chunk()
    {
        m_chunks.emplace_back();
        return --m_chunks.end();
    }

    typename list_t::iterator create_chunk(size_t cell_size)
    {
        auto alloc_res = allocate_block(cell_size);
        m_chunks.emplace(std::prev(m_chunks.end()), alloc_res.first, alloc_res.second, cell_size);
        return --std::prev(m_chunks.end());
    }

    typename list_t::iterator destroy_chunk(typename list_t::iterator chk)
    {
        m_cache.invalidate(chk, std::next(chk));
        deallocate_block(chk->get_mem(), chk->get_mem_size());
        return m_chunks.erase(chk);
    }

    std::pair<internal_pointer_type, size_t> allocate_block(size_t cell_size)
    {
        assert(PAGE_SIZE % cell_size == 0);
        size_t chunk_size = Chunk::chunk_size(cell_size);
        return std::make_pair(upstream_allocate(chunk_size), chunk_size);
    }

    void deallocate_block(internal_pointer_type p, size_t size)
    {
        assert(p);
        upstream_deallocate(p, size);
    }

    byte* upstream_allocate(size_t size)
    {
        return this->template get_base<UpstreamAlloc>().allocate(size);
    }

    void upstream_deallocate(byte* ptr, size_t size)
    {
        this->template get_base<UpstreamAlloc>().deallocate(ptr, size);
    }

    list_t m_chunks;
    cache_t m_cache;
    mutable Lock m_lock;
};

}}}

#endif //DIPLOMA_FIXED_SIZE_ALLOCATOR_H
