#include <libprecisegc/details/threads/static_root_set.hpp>

namespace precisegc { namespace details { namespace threads {

static_root_set static_root_set::root_set{};

void static_root_set::register_root(gc_handle* root)
{
    root_set.insert(root);
}

void static_root_set::deregister_root(gc_handle* root)
{
    root_set.remove(root);
}

bool static_root_set::is_root(const gc_handle* ptr)
{
    return root_set.contains(ptr);
}

size_t static_root_set::count()
{
    return root_set.size();
}

void static_root_set::insert(gc_handle* ptr)
{
    gc_unsafe_scope unsafe_scope;
    std::lock_guard<mutex_t> lock_guard(m_lock);
    m_set.insert(ptr);
}

void static_root_set::remove(gc_handle* ptr)
{
    gc_unsafe_scope unsafe_scope;
    std::lock_guard<mutex_t> lock_guard(m_lock);
    m_set.erase(ptr);
}

bool static_root_set::contains(const gc_handle* ptr)
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

}}}
