#ifndef DIPLOMA_UNORDERED_POINTER_SET_HPP
#define DIPLOMA_UNORDERED_POINTER_SET_HPP

#include <mutex>
#include <unordered_set>

#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

template <typename T, typename Lock>
class unordered_pointer_set : private utils::noncopyable, private utils::nonmovable
{
public:
    unordered_pointer_set() = default;

    void insert(T* ptr)
    {
        gc_unsafe_scope unsafe_scope;
        std::lock_guard<Lock> lock_guard(m_lock);
        m_set.insert(ptr);
    }

    void remove(T* ptr)
    {
        gc_unsafe_scope unsafe_scope;
        std::lock_guard<Lock> lock_guard(m_lock);
        m_set.erase(ptr);
    }

    bool contains(const T* ptr)
    {
        gc_unsafe_scope unsafe_scope;
        std::lock_guard<Lock> lock_guard(m_lock);
        return m_set.count(const_cast<T*>(ptr));
    }

    template <typename Functor>
    void trace(Functor&& f) const
    {
        std::for_each(m_set.begin(), m_set.end(), std::forward<Functor>(f));
    }

    size_t count() const
    {
        gc_unsafe_scope unsafe_scope;
        std::lock_guard<Lock> lock_guard(m_lock);
        return m_set.size();
    }
private:
    std::unordered_set<T*> m_set;
    Lock m_lock;
};

}}}

#endif //DIPLOMA_UNORDERED_POINTER_SET_HPP
