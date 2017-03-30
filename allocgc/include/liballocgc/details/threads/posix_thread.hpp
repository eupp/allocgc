#ifndef ALLOCGC_POSIX_THREAD_HPP
#define ALLOCGC_POSIX_THREAD_HPP

#include <type_traits>
#include <thread>
#include <pthread.h>

namespace allocgc { namespace details { namespace threads {

static_assert(std::is_same<std::thread::native_handle_type, pthread_t>::value, "Only pthreads are supported");

inline std::thread::native_handle_type this_thread_native_handle()
{
    return pthread_self();
}

}}}

#endif //ALLOCGC_POSIX_THREAD_HPP
