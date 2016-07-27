#ifndef DIPLOMA_THIS_MANAGED_THREAD_HPP
#define DIPLOMA_THIS_MANAGED_THREAD_HPP

#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include "managed_thread.hpp"

namespace precisegc { namespace details { namespace threads {

class managed_thread;
class managed_thread_accessor;

class this_managed_thread : private utils::noncopyable, private utils::nonmovable
{
public:
    this_managed_thread() = delete;

    // for testing purpose
    static managed_thread* thread_ptr();

    static std::thread::id get_id();
    static std::thread::native_handle_type get_native_handle();

    static void register_root(ptrs::gc_untyped_ptr* root);
    static void deregister_root(ptrs::gc_untyped_ptr* root);

    static void pin(byte* ptr);
    static void unpin(byte* ptr);

    static std::unique_ptr<collectors::mark_packet>& get_mark_packet();

    friend class managed_thread_accessor;
private:
    static thread_local managed_thread* this_thread;
};

}}}

#endif //DIPLOMA_THIS_MANAGED_THREAD_HPP
