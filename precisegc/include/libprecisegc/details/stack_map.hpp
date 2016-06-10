#ifndef DIPLOMA_ROOT_SET_HPP
#define DIPLOMA_ROOT_SET_HPP

#include <algorithm>
#include <forward_list>

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

template <typename T>
class stack_map : private utils::noncopyable, private utils::nonmovable
{
public:
    stack_map()
        : m_head(nullptr)
    {};

    void insert(T elem)
    {
        gc_unsafe_scope unsafe_scope;
        push_front(elem);
    }

    void remove(T elem)
    {
        gc_unsafe_scope unsafe_scope;
        remove_first(elem);
    }

    bool contains(T elem)
    {
        gc_unsafe_scope unsafe_scope;
        return std::find(begin(), end(), elem) != end();
    }

    template <typename Functor>
    void trace(Functor& f) const
    {
        std::for_each(begin(), end(), f);
    }

    template <typename Functor>
    void trace(const Functor& f) const
    {
        std::for_each(begin(), end(), f);
    }
private:
    struct node
    {
        T m_value;
        node* m_next;
    };

    class iterator: public boost::iterator_facade<
              iterator
            , const T
            , boost::forward_traversal_tag
    >
    {
    public:
        iterator(node* pnode) noexcept
            : m_pnode(pnode)
        {}

        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        const T* operator->() const
        {
            return &m_pnode->m_value;
        }
    private:
        friend class stack_map;
        friend class boost::iterator_core_access;

        const T& dereference() const
        {
            return m_pnode->m_value;
        }

        void increment() noexcept
        {
            m_pnode = m_pnode->m_next;
        }

        bool equal(const iterator& other) const noexcept
        {
            return m_pnode == other.m_pnode;
        }

        node* m_pnode;
    };

    iterator begin() const
    {
        return iterator(m_head);
    }

    iterator end() const
    {
        return iterator(nullptr);
    }

    void push_front(T elem)
    {
        node* pnode = create_node(elem);
        pnode->m_next = m_head;
        m_head = pnode;
    }

    void remove_first(T elem)
    {
        assert(m_head);
        if (m_head->m_value == elem) {
            node* tmp = m_head;
            m_head = m_head->m_next;
            destroy_node(tmp);
            return;
        }
        node* it = m_head;
        node* next = it->m_next;
        while (next && next->m_value != elem) {
            it = next;
            next = next->m_next;
        }
        if (next) {
            it->m_next = next->m_next;
            destroy_node(next);
        }
    }

    node* create_node(T elem)
    {
        node* pnode = reinterpret_cast<node*>(m_pool.allocate(sizeof(node)));
        pnode->m_value = elem;
        return pnode;
    }

    void destroy_node(node* pnode)
    {
        m_pool.deallocate(reinterpret_cast<byte*>(pnode), sizeof(node));
    }

    typedef allocators::intrusive_list_allocator<
            allocators::freelist_pool_chunk, allocators::default_allocator
        > object_pool_t;

    node* m_head;
    object_pool_t m_pool;
};

typedef stack_map<ptrs::gc_untyped_ptr*> root_stack_map;
typedef stack_map<byte*> pin_stack_map;

}}

#endif //DIPLOMA_ROOT_SET_HPP
