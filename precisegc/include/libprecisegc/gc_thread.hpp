#ifndef DIPLOMA_GC_THREAD_HPP
#define DIPLOMA_GC_THREAD_HPP

#include <thread>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/threads/return_address.hpp>

namespace precisegc {

class gc_thread
{
public:
    template <typename Function, typename... Args>
    static std::thread create(Function&& f, Args&&... args)
    {
        typedef decltype(std::bind(std::forward<Function>(f), std::forward<Args>(args)...)) functor_type;
        std::unique_ptr<functor_type> pf(
                new functor_type(std::bind(std::forward<Function>(f), std::forward<Args>(args)...))
        );
        return std::thread(&start_routine<functor_type>, std::move(pf));
    };
private:
    template <typename Functor>
    static void start_routine(std::unique_ptr<Functor> bf)
    {
        using namespace details;
        using namespace details::threads;

        gc_register_thread(std::this_thread::get_id(), frame_address());
        (*bf)();
        gc_deregister_thread(std::this_thread::get_id());
    }
};

}

#endif //DIPLOMA_GC_THREAD_HPP
