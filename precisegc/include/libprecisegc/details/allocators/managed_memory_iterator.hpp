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

    managed_memory_proxy(byte* ptr, MemoryDescriptor* descr)
        : m_ptr(ptr)
        , m_descr(descr)
    {
        assert(ptr && descr);
    }

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

    size_t cell_size() const
    {
        return m_descr->cell_size(m_ptr);
    }

    void destroy()
    {
        m_descr->destroy(m_ptr);
    }

    void move(const managed_memory_proxy& to)
    {
        m_descr->move(m_ptr, to.m_ptr);
    }

    friend class managed_memory_iterator<MemoryDescriptor>;
private:
    byte* m_ptr;
    MemoryDescriptor* m_descr;
};

template <typename MemoryDescriptor>
class managed_memory_iterator
{
    typedef managed_memory_proxy<MemoryDescriptor> proxy_t;
public:
    typedef const proxy_t         value_type;
    typedef ptrdiff_t             difference_type;
    typedef const proxy_t*        pointer;
    typedef const proxy_t         reference;

    managed_memory_iterator() noexcept = default;
    managed_memory_iterator(const managed_memory_iterator&) noexcept = default;
    managed_memory_iterator(managed_memory_iterator&&) noexcept = default;

    managed_memory_iterator& operator=(const managed_memory_iterator&) noexcept = default;
    managed_memory_iterator& operator=(managed_memory_iterator&&) noexcept = default;

    const proxy_t* operator->()
    {
        return &m_proxy;
    }
protected:
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

    byte* get_ptr() const
    {
        return m_proxy.m_ptr;
    }

    void  set_ptr(byte* ptr)
    {
        m_proxy.m_ptr = ptr;
    }

    MemoryDescriptor* get_descriptor() const
    {
        return m_proxy.m_descr;
    }

    void set_descriptor(MemoryDescriptor* descr)
    {
        m_proxy.m_descr = descr;
    }

    proxy_t m_proxy;
};

}}}

#endif //DIPLOMA_MANAGED_PTR_ITERATOR_HPP
