#ifndef ALLOCGC_GC_H
#define ALLOCGC_GC_H

#include <utility>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/gc_ptr.hpp>
#include <liballocgc/gc_pin.hpp>
#include <liballocgc/gc_ref.hpp>
#include <liballocgc/gc_new.hpp>
#include <liballocgc/details/collectors/gc_serial.hpp>

namespace allocgc {

int gc_init(const gc_params& params = gc_params());

void gc();

gc_stat gc_stats();

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

}

}

#endif //ALLOCGC_GC_H
