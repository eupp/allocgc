#ifndef DIPLOMA_ROOT_SET_HPP
#define DIPLOMA_ROOT_SET_HPP

#include <atomic>
#include <algorithm>

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/allocators/intrusive_list_pool_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace threads {

template <typename T>
class stack_map : private utils::noncopyable, private utils::nonmovable
{
public:
    stack_map()
        : m_head(nullptr)
        , m_free_node_cnt(0)
    {};

    void insert(T elem)
    {
        push_front(elem);
    }

    void remove(T elem)
    {
        remove_first(elem);
    }

    bool contains(T elem)
    {
        return std::find(begin(), end(), elem) != end();
    }

    template <typename Functor>
    void trace(Functor&& f) const
    {
        std::for_each(begin(), end(), f);
    }
private:
    static const size_t MAX_FREE_NODE = 4096;

    struct node
    {
        T m_value;
        std::atomic<node*> m_next;
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
            m_pnode = m_pnode->m_next.load(std::memory_order_relaxed);
        }

        bool equal(const iterator& other) const noexcept
        {
            return m_pnode == other.m_pnode;
        }

        node* m_pnode;
    };

    iterator begin() const
    {
        return iterator(m_head.load(std::memory_order_relaxed));
    }

    iterator end() const
    {
        return iterator(nullptr);
    }

    void push_front(T elem)
    {
        node* pnode = create_node(elem);
        pnode->m_next.store(m_head.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_head.store(pnode, std::memory_order_relaxed);
    }

    void remove_first(T elem)
    {
        assert(m_head);
        node* head = m_head.load(std::memory_order_relaxed);
        if (head->m_value == elem) {
            node* tmp = head;
            m_head.store(head->m_next.load(std::memory_order_relaxed), std::memory_order_relaxed);
            destroy_node(tmp);
            return;
        }
        node* it = head;
        node* next = it->m_next.load(std::memory_order_relaxed);
        while (next && next->m_value != elem) {
            it = next;
            next = next->m_next.load(std::memory_order_relaxed);
        }
        if (next) {
            it->m_next.store(next->m_next.load(std::memory_order_relaxed), std::memory_order_relaxed);
            destroy_node(next);
            return;
        }
        assert(false);
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
        if (++m_free_node_cnt == MAX_FREE_NODE) {
            shrink_pool();
            m_free_node_cnt = 0;
        }
    }

    void shrink_pool()
    {
        m_pool.shrink(sizeof(node));
    }

    typedef allocators::intrusive_list_pool_allocator<
            allocators::freelist_pool_chunk, allocators::default_allocator
        > object_pool_t;

    std::atomic<node*> m_head;
    object_pool_t m_pool;
    size_t m_free_node_cnt;
};

typedef stack_map<ptrs::gc_untyped_ptr*> root_stack_map;
typedef stack_map<byte*> pin_stack_map;

}}}

#endif //DIPLOMA_ROOT_SET_HPP
