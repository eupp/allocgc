#ifndef DIPLOMA_STATIC_ROOT_SET_HPP
#define DIPLOMA_STATIC_ROOT_SET_HPP

#include <mutex>
#include <utility>
#include <algorithm>
#include <unordered_set>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class static_root_set : private utils::noncopyable, private utils::nonmovable
{
public:
    static void register_root(gc_handle* root);
    static void deregister_root(gc_handle* root);

    static bool is_root(const gc_handle* ptr);

    template <typename Functor>
    static void trace(Functor&& f)
    {
        std::for_each(root_set.m_set.begin(), root_set.m_set.end(), std::forward<Functor>(f));
    }

    static size_t count();
private:
    typedef std::mutex mutex_t;

    static_root_set() = default;

    void insert(gc_handle* ptr);
    void remove(gc_handle* ptr);

    bool contains(const gc_handle* ptr);

    size_t size() const;

    static static_root_set root_set;

    std::unordered_set<gc_handle*> m_set;
    mutable mutex_t m_lock;
};

}}}

#endif //DIPLOMA_STATIC_ROOT_SET_HPP
