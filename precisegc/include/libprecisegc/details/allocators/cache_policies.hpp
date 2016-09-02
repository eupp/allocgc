#ifndef DIPLOMA_CACHE_POLICIES_HPP
#define DIPLOMA_CACHE_POLICIES_HPP

#include <cstddef>
#include <cassert>
#include <utility>

#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename Container>
class single_chunk_cache
{
public:
    typedef typename Container::value_type chunk_type;
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    single_chunk_cache()
        : m_cache(m_chunks.begin())
    {}

    template <typename Alloc>
    iterator destroy(Alloc& alloc, iterator it)
    {
        return destroy_chunk(alloc, it);
    }

    iterator begin()
    {
        return m_chunks.begin();
    }

    const_iterator begin() const
    {
        return m_chunks.begin();
    }

    const_iterator cbegin() const
    {
        return m_chunks.cbegin();
    }

    iterator end()
    {
        return m_chunks.end();
    }

    const_iterator end() const
    {
        return m_chunks.end();
    }

    const_iterator cend() const
    {
        return m_chunks.cend();
    }
private:
    template <typename Alloc>
    typename iterator create_chunk(Alloc& alloc, size_t cell_size)
    {
        auto alloc_res = allocate_blk(alloc, cell_size);
        m_chunks.emplace_back(alloc_res.first, alloc_res.second, cell_size);
        return --m_chunks.end();
    }

    template <typename Alloc>
    typename iterator destroy_chunk(Alloc& alloc, iterator chk)
    {
        if (m_cache == chk) {
            m_cache++;
        }
        deallocate_blk(alloc, chk->get_mem(), chk->get_mem_size());
        return m_chunks.erase(chk);
    }

    template <typename Alloc>
    std::pair<typename Alloc::pointer_type, size_t> allocate_blk(Alloc& alloc, size_t cell_size)
    {
        assert(PAGE_SIZE % cell_size == 0);
        size_t chunk_size = chunk_type::get_chunk_size(cell_size);
        return std::make_pair(alloc.allocate(chunk_size, chunk_size), chunk_size);
    }

    template <typename Alloc>
    void deallocate_blk(Alloc& alloc, typename Alloc::pointer_type ptr, size_t size)
    {
        assert(ptr);
        alloc.deallocate(ptr, size, size);
    }

    Container m_chunks;
    iterator m_cache;
};

}}}

#endif //DIPLOMA_CACHE_POLICIES_HPP
