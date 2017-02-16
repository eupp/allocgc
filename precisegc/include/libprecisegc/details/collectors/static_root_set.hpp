#ifndef DIPLOMA_STATIC_ROOT_SET_HPP
#define DIPLOMA_STATIC_ROOT_SET_HPP

#include <mutex>
#include <utility>
#include <algorithm>
#include <unordered_set>

#include <libprecisegc/gc_handle.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

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

#endif //DIPLOMA_STATIC_ROOT_SET_HPP
