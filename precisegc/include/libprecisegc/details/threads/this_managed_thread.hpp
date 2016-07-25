#ifndef DIPLOMA_THIS_MANAGED_THREAD_HPP
#define DIPLOMA_THIS_MANAGED_THREAD_HPP

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/managed_ptr.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread;
class managed_thread_accessor;

class this_managed_thread : private utils::noncopyable, private utils::nonmovable
{
public:
    this_managed_thread() = delete;

    void register_root(ptrs::gc_untyped_ptr* root);
    void deregister_root(ptrs::gc_untyped_ptr* root);

    void pin(byte* ptr);
    void unpin(byte* ptr);

    friend class managed_thread_accessor;
private:
    static thread_local managed_thread* this_thread;
};

}}}

#endif //DIPLOMA_THIS_MANAGED_THREAD_HPP
