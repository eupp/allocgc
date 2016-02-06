#ifndef GC_NEW_H
#define GC_NEW_H

#include <cstdio>
#include <cassert>
#include <vector>
#include <type_traits>
#include <pthread.h>

#include "gc_ptr.h"
#include "debug_print.h"
#include "thread.h"
#include "tlvars.h"
#include "details/class_meta.h"
#include "details/gc_pause.h"
#include "details/gc_new_impl.h"

namespace precisegc {

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> typename details::gc_new_if<T>::single_object
{

};

template <typename T>
auto gc_new(size_t n)
    -> typename details::gc_new_if<T>::unknown_bound
{
    typedef typename std::remove_extent<T>::type U;
};

template<typename T, typename... Args>
auto gc_new(Args&&...)
    -> typename details::gc_new_if<T>::known_bound
    = delete;

}

#endif //GC_NEW_H