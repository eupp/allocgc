#ifndef DIPLOMA_ROOT_SET_HPP
#define DIPLOMA_ROOT_SET_HPP

#include <atomic>
#include <algorithm>
#include <utility>

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/allocators/pool.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc { namespace details { namespace collectors {

class pin_set : private utils::noncopyable, private utils::nonmovable
{
public:
    pin_set()
        : m_head(nullptr)
        , m_pool()
    {};

    void insert(byte* elem)
    {
        push_front(elem);
    }

    void remove(byte* elem)
    {
        remove_first(elem);
    }

    bool contains(byte* elem) const
    {
        return std::find(begin(), end(), elem) != end();
    }

    void trace(const gc_trace_pin_callback& cb) const
    {
        std::for_each(begin(), end(), cb);
    }

    size_t size() const
    {
        return std::distance(begin(), end());
    }
private:
    static const size_t MAX_FREE_NODE = 4096;

    struct node
    {
        byte* m_value;
        std::atomic<node*> m_next;
    };

    class iterator: public boost::iterator_facade<
              iterator
            , byte* const
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
    private:
        friend class pin_set;
        friend class boost::iterator_core_access;

        byte* const& dereference() const
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

    void push_front(byte* elem)
    {
        node* pnode = create_node(elem);
        pnode->m_next.store(m_head.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_head.store(pnode, std::memory_order_relaxed);
    }

    void remove_first(byte* elem)
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

    node* create_node(byte* elem)
    {
        node* pnode = m_pool.create();
        pnode->m_value = elem;
        return pnode;
    }

    void destroy_node(node* pnode)
    {
        m_pool.destroy(pnode);
    }

    std::atomic<node*> m_head;
    allocators::pool<node, utils::dummy_mutex> m_pool;
};

}}}

#endif //DIPLOMA_ROOT_SET_HPP
