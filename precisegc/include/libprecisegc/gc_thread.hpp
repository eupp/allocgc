#ifndef DIPLOMA_GC_THREAD_HPP
#define DIPLOMA_GC_THREAD_HPP

#include <thread>

#include <libprecisegc/details/threads/managed_thread.hpp>

namespace precisegc {

class gc_thread
{
public:
    template <typename Function, typename... Args>
    static std::thread create(Function&& f, Args&&... args)
    {
        return ::precisegc::details::threads::managed_thread::create(std::forward<Function>(f),
                                                                     std::forward<Args>(args)...);
    };
};

}

#endif //DIPLOMA_GC_THREAD_HPP
