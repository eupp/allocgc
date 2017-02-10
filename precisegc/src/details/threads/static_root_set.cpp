#include <libprecisegc/details/collectors/static_root_set.hpp>

namespace precisegc { namespace details { namespace collectors {

void static_root_set::register_root(gc_handle* root)
{
    gc_unsafe_scope unsafe_scope;
    std::lock_guard<mutex_t> lock_guard(m_lock);
    m_set.insert(root);
}

void static_root_set::deregister_root(gc_handle* root)
{
    gc_unsafe_scope unsafe_scope;
    std::lock_guard<mutex_t> lock_guard(m_lock);
    m_set.erase(root);
}

bool static_root_set::is_root(const gc_handle* ptr) const
{
    gc_unsafe_scope unsafe_scope;
    std::lock_guard<mutex_t> lock_guard(m_lock);
    return m_set.count(const_cast<gc_handle*>(ptr));
}

size_t static_root_set::size() const
{
    gc_unsafe_scope unsafe_scope;
    std::lock_guard<mutex_t> lock_guard(m_lock);
    return m_set.size();
}

void static_root_set::trace(const gc_trace_callback& cb) const
{
    std::lock_guard<mutex_t> lock_guard(m_lock);
    std::for_each(m_set.begin(), m_set.end(), cb);
}

}}}
