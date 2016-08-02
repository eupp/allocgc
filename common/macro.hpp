#ifndef DIPLOMA_MACRO_HPP
#define DIPLOMA_MACRO_HPP

#ifdef PRECISE_GC
    #include "libprecisegc/libprecisegc.hpp"
#endif

#ifdef BDW_GC
    #define GC_THREADS
    #include <gc/gc.h>
#endif

#ifdef SHARED_PTR
    #include <memory>
#endif

#if defined(PRECISE_GC)
    #define ptr_t(T) precisegc::gc_ptr<T>
    #define ptr_in(T) const precisegc::gc_ptr<T>&
    #define new_(T) precisegc::gc_new<T>()
    #define new_args_(T, ...) precisegc::gc_new<T>(__VA_ARGS__)
    #define ptr_array_t(T) precisegc::gc_ptr<T[]>
    #define new_array_(T, size) precisegc::gc_new<T[]>(size)
    #define delete_(ptr)
    #define set_null(ptr) ptr.reset()
    #define null_ptr(T) precisegc::gc_ptr<T>()
#elif defined(BDW_GC)
    #define ptr_t(T) T*
    #define ptr_in(T) T*
    #define new_(T) new (GC_NEW(T)) T()
    #define new_args_(T, ...) new (GC_NEW(T)) T(__VA_ARGS__)
    #define ptr_array_t(T) T*
    #define new_array_(T, size) (T*) GC_MALLOC_ATOMIC(sizeof(T) * size);
    #define delete_(ptr)
    #define set_null(ptr) ptr = nullptr
    #define null_ptr(T) nullptr
#elif defined(SHARED_PTR)
    #define ptr_t(T) std::shared_ptr<T>
    #define ptr_in(T) const std::shared_ptr<T>&
    #define new_(T) std::make_shared<T>()
    #define new_args_(T, ...) std::make_shared<T>(__VA_ARGS__)
    #define ptr_array_t(T) std::shared_ptr<T, std::default_delete<T[]>>
    #define new_array_(T, size) std::shared_ptr<T>(new T[size], std::default_delete<T[]>())
    #define delete_(ptr)
    #define set_null(ptr) ptr.reset()
    #define null_ptr(T) std::shared_ptr<T>()
#elif defined(NO_GC)
    #define ptr_t(T) T*
    #define ptr_in(T) T*
    #define new_(T) new T()
    #define new_args_(T, ...) new T(__VA_ARGS__)
    #define ptr_array_t(T) T*
    #define new_array_(T, size) new T[kArraySize]
    #define delete_(ptr) if (ptr) delete ptr
    #define set_null(ptr) ptr = nullptr
    #define null_ptr(T) nullptr
#endif

#endif //DIPLOMA_MACRO_HPP
