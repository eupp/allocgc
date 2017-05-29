#ifndef ALLOCGC_GC_H
#define ALLOCGC_GC_H

#include <utility>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/gc_ptr.hpp>
#include <liballocgc/gc_pin.hpp>
#include <liballocgc/gc_ref.hpp>
#include <liballocgc/gc_new.hpp>
#include <liballocgc/details/threads/posix_thread.hpp>
#include <liballocgc/details/threads/return_address.hpp>
#include <liballocgc/details/collectors/gc_cms.hpp>
#include <liballocgc/details/collectors/gc_serial.hpp>


namespace allocgc {

void enable_logging(gc_loglevel loglevel);

namespace serial {

template <typename T>
using gc_ptr = pointers::gc_ptr<T, details::collectors::gc_serial>;

template <typename T>
using gc_pin = pointers::gc_pin<T, details::collectors::gc_serial>;

template <typename T>
using gc_ref = pointers::gc_ref<T, details::collectors::gc_serial>;

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> decltype(pointers::gc_new<T, details::collectors::gc_serial>(std::forward<Args>(args)...))
{
    return pointers::gc_new<T, details::collectors::gc_serial>(std::forward<Args>(args)...);
};

template <typename T>
auto gc_new(size_t n)
    -> decltype(pointers::gc_new<T, details::collectors::gc_serial>(n))
{
    return pointers::gc_new<T, details::collectors::gc_serial>(n);
};

void gc();

gc_stat stats();

void set_heap_limit(size_t limit);

void register_main_thread();
void register_thread(const thread_descriptor& descr);
void deregister_thread(std::thread::id id);

template <typename F, typename... Args>
std::thread create_thread(F&& f, Args&&... args)
{
    typedef decltype(std::bind(std::forward<F>(f), std::forward<Args>(args)...)) functor_type;

    return std::thread([] (std::unique_ptr<functor_type> bf) {

        thread_descriptor thrd_descr;
        thrd_descr.id = std::this_thread::get_id();
        thrd_descr.native_handle = details::threads::this_thread_native_handle();
        thrd_descr.stack_start_addr = details::threads::frame_address();

        register_thread(thrd_descr);
        (*bf)();
        deregister_thread(std::this_thread::get_id());
    }, std::unique_ptr<functor_type>(new functor_type(std::bind(std::forward<F>(f), std::forward<Args>(args)...))));
};

}

namespace cms {

template <typename T>
using gc_ptr = pointers::gc_ptr<T, details::collectors::gc_cms>;

template <typename T>
using gc_pin = pointers::gc_pin<T, details::collectors::gc_cms>;

template <typename T>
using gc_ref = pointers::gc_ref<T, details::collectors::gc_cms>;

template <typename T, typename... Args>
auto gc_new(Args&&... args)
-> decltype(pointers::gc_new<T, details::collectors::gc_cms>(std::forward<Args>(args)...))
{
    return pointers::gc_new<T, details::collectors::gc_cms>(std::forward<Args>(args)...);
};

template <typename T>
auto gc_new(size_t n)
-> decltype(pointers::gc_new<T, details::collectors::gc_cms>(n))
{
    return pointers::gc_new<T, details::collectors::gc_cms>(n);
};

void gc();

gc_stat stats();

void set_heap_limit(size_t limit);

void register_main_thread();
void register_thread(const thread_descriptor& descr);
void deregister_thread(std::thread::id id);

template <typename F, typename... Args>
std::thread create_thread(F&& f, Args&&... args)
{
    typedef decltype(std::bind(std::forward<F>(f), std::forward<Args>(args)...)) functor_type;

    return std::thread([] (std::unique_ptr<functor_type> bf) {

        thread_descriptor thrd_descr;
        thrd_descr.id = std::this_thread::get_id();
        thrd_descr.native_handle = details::threads::this_thread_native_handle();
        thrd_descr.stack_start_addr = details::threads::frame_address();

        register_thread(thrd_descr);
        (*bf)();
        deregister_thread(std::this_thread::get_id());
    }, std::unique_ptr<functor_type>(new functor_type(std::bind(std::forward<F>(f), std::forward<Args>(args)...))));
};

}

}

#endif //ALLOCGC_GC_H
