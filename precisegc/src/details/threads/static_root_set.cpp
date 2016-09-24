#include <libprecisegc/details/threads/static_root_set.hpp>

namespace precisegc { namespace details { namespace threads {

unordered_pointer_set<gc_handle, std::mutex> static_root_set::root_set{};

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
    return root_set.count();
}

}}}
