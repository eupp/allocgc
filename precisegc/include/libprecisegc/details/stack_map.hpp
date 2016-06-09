#ifndef DIPLOMA_ROOT_SET_HPP
#define DIPLOMA_ROOT_SET_HPP

#include <algorithm>
#include <forward_list>

#include <libprecisegc/details/allocators/object_pool.hpp>
#include <libprecisegc/details/allocators/stl_adapter.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

template <typename T>
class stack_map : private utils::noncopyable, private utils::nonmovable
{
public:
    stack_map() = default;

    void insert(T elem)
    {
        gc_unsafe_scope unsafe_scope;
        m_list.push_front(elem);
    }

    void remove(T elem)
    {
        gc_unsafe_scope unsafe_scope;
        remove_first(elem);
    }

    bool contains(T elem)
    {
        gc_unsafe_scope unsafe_scope;
        return std::find(m_list.begin(), m_list.end(), elem) != m_list.end();
    }

    template <typename Functor>
    void trace(Functor& f) const
    {
        std::for_each(m_list.begin(), m_list.end(), f);
    }

    template <typename Functor>
    void trace(const Functor& f) const
    {
        std::for_each(m_list.begin(), m_list.end(), f);
    }
private:
    void remove_first(T elem)
    {
        auto it = m_list.before_begin();
        auto next = std::next(it);
        auto last = m_list.end();
        while (next != last && *next != elem) {
            it = next;
            ++next;
        }
        if (next != last) {
            m_list.erase_after(it);
        }
    }

    std::forward_list<T, allocators::stl_adapter<T, allocators::object_pool>> m_list;
};

typedef stack_map<ptrs::gc_untyped_ptr*> root_stack_map;
typedef stack_map<byte*> pin_stack_map;

}}

#endif //DIPLOMA_ROOT_SET_HPP
