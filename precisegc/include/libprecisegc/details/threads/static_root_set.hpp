#ifndef DIPLOMA_STATIC_ROOT_SET_HPP
#define DIPLOMA_STATIC_ROOT_SET_HPP

#include <utility>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/threads/unordered_pointer_set.hpp>
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
        root_set.trace(std::forward<Functor>(f));
    }

    static size_t count();
private:
    static_root_set() = delete;

    static unordered_pointer_set<gc_handle, std::mutex> root_set;
};

}}}

#endif //DIPLOMA_STATIC_ROOT_SET_HPP
