#ifndef ALLOCGC_STATIC_ROOT_SET_HPP
#define ALLOCGC_STATIC_ROOT_SET_HPP

#include <mutex>
#include <utility>
#include <algorithm>
#include <unordered_set>

#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace collectors {

class static_root_set : private utils::noncopyable, private utils::nonmovable
{
public:
    static_root_set() = default;

    void register_root(gc_handle* root);
    void deregister_root(gc_handle* root);

    bool is_root(const gc_handle* ptr) const;

    size_t size() const;

    void trace(const gc_trace_callback& cb) const;
private:
    typedef std::mutex mutex_t;

    std::unordered_set<gc_handle*> m_set;
    mutable mutex_t m_lock;
};

}}}

#endif //ALLOCGC_STATIC_ROOT_SET_HPP
