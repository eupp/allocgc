#ifndef DIPLOMA_MANAGED_PTR_ITERATOR_HPP
#define DIPLOMA_MANAGED_PTR_ITERATOR_HPP

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/collectors/indexed_managed_object.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_pool_chunk;
class managed_object_descriptor;

template <typename Chunk>
class managed_ptr_iterator : public boost::iterator_facade<
          managed_ptr_iterator<Chunk>
        , const indexed_managed_object
        , boost::random_access_traversal_tag
        , const indexed_managed_object
    >
{
public:
    managed_ptr_iterator() noexcept = default;
    managed_ptr_iterator(const managed_ptr_iterator&) noexcept = default;
    managed_ptr_iterator(managed_ptr_iterator&&) noexcept = default;

    managed_ptr_iterator& operator=(const managed_ptr_iterator&) noexcept = default;
    managed_ptr_iterator& operator=(managed_ptr_iterator&&) noexcept = default;

    const indexed_managed_object* operator->()
    {
        return &m_ptr;
    }
private:
    friend class managed_pool_chunk;
    friend class managed_object_descriptor;
    friend class boost::iterator_core_access;

    managed_ptr_iterator(byte* ptr, memory_descriptor* descr) noexcept
        : m_ptr(ptr, descr)
    {}

    indexed_managed_object dereference() const
    {
        return m_ptr;
    }

    void increment() noexcept
    {
        m_ptr.advance(cell_size());
    }

    void decrement() noexcept
    {
        m_ptr.advance(-cell_size());
    }

    bool equal(const managed_ptr_iterator& other) const noexcept
    {
        return m_ptr.get() == other.m_ptr.get();
    }

    void advance(ptrdiff_t n)
    {
        m_ptr.advance(n * cell_size());
    }

    ptrdiff_t distance_to(const managed_ptr_iterator& other) const
    {
        return other.m_ptr.get() - m_ptr.get();
    }

    size_t cell_size() const
    {
        Chunk* chunk = static_cast<Chunk*>(m_ptr.get_descriptor());
        return chunk->cell_size();
    }

    indexed_managed_object m_ptr;
};

}}}

#endif //DIPLOMA_MANAGED_PTR_ITERATOR_HPP
