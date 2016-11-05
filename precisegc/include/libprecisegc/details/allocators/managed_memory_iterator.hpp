#ifndef DIPLOMA_MANAGED_PTR_ITERATOR_HPP
#define DIPLOMA_MANAGED_PTR_ITERATOR_HPP

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/collectors/indexed_managed_object.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_pool_chunk;
class managed_object_descriptor;

template <typename MemoryDescriptor>
class managed_memory_iterator;

template<typename MemoryDescriptor>
class managed_memory_proxy
{
public:
    managed_memory_proxy(const managed_memory_proxy&) = default;

    managed_memory_proxy& operator=(const managed_memory_proxy&) = default;

    managed_memory_proxy() noexcept
        : m_ptr(nullptr)
        , m_descr(nullptr)
    {}

    byte* get() const noexcept
    {
        return m_ptr;
    }

    bool get_mark() const
    {
        return m_descr->get_mark(m_ptr);
    }

    bool get_pin() const
    {
        return m_descr->get_pin(m_ptr);
    }

    void set_mark(bool mark) const
    {
        return m_descr->set_mark(m_ptr, mark);
    }

    void set_pin(bool pin) const
    {
        return m_descr->set_pin(m_ptr, pin);
    }

    bool is_dead() const
    {
        return m_descr->is_dead(m_ptr);
    }

    void set_dead() const
    {
        return m_descr->set_dead(m_ptr);
    }

    size_t cell_size() const
    {
        return m_descr->cell_size();
    }

    friend class managed_memory_iterator<MemoryDescriptor>;
private:
    managed_memory_proxy(byte* ptr, MemoryDescriptor* descr)
            : m_ptr(ptr)
              , m_descr(descr)
    {
        assert(ptr && descr);
    }

    byte* m_ptr;
    MemoryDescriptor* m_descr;
};

template <typename MemoryDescriptor>
class managed_memory_iterator : public boost::iterator_facade<
          managed_memory_iterator<MemoryDescriptor>
        , const managed_memory_proxy<MemoryDescriptor>
        , boost::random_access_traversal_tag
        , const managed_memory_proxy<MemoryDescriptor>
    >
{
    typedef managed_memory_proxy<MemoryDescriptor> proxy_t;
public:
    managed_memory_iterator() noexcept = default;
    managed_memory_iterator(const managed_memory_iterator&) noexcept = default;
    managed_memory_iterator(managed_memory_iterator&&) noexcept = default;

    managed_memory_iterator& operator=(const managed_memory_iterator&) noexcept = default;
    managed_memory_iterator& operator=(managed_memory_iterator&&) noexcept = default;

    const proxy_t* operator->()
    {
        return &m_proxy;
    }
private:
    friend class managed_pool_chunk;
    friend class managed_object_descriptor;
    friend class boost::iterator_core_access;

    managed_memory_iterator(byte* ptr, MemoryDescriptor* descr) noexcept
        : m_proxy(ptr, descr)
    {}

    proxy_t dereference() const
    {
        return m_proxy;
    }

    void increment() noexcept
    {
        advance(1);
    }

    void decrement() noexcept
    {
        advance(-1);
    }

    bool equal(const managed_memory_iterator& other) const noexcept
    {
        return m_proxy.get() == other.m_proxy.get();
    }

    void advance(ptrdiff_t n)
    {
        m_proxy.m_ptr += n * cell_size();
    }

    ptrdiff_t distance_to(const managed_memory_iterator& other) const
    {
        return other.m_proxy.get() - m_proxy.get();
    }

    size_t cell_size() const
    {
        return m_proxy.cell_size();
    }

    proxy_t m_proxy;
};

}}}

#endif //DIPLOMA_MANAGED_PTR_ITERATOR_HPP
