#ifndef DIPLOMA_GC_PTR_H
#define DIPLOMA_GC_PTR_H

#include <cstdint>
#include <utility>
#include <iterator>

#include "gc_new.h"
#include "libprecisegc/details/ptrs/gc_untyped_ptr.hpp"
#include "libprecisegc/details/ptrs/gc_untyped_pin.hpp"
#include "libprecisegc/details/ptrs/gc_ptr_access.hpp"
#include "details/gc_unsafe_scope.h"
#include "details/iterator_facade.h"
#include "details/iterator_access.h"

namespace precisegc {

namespace details { namespace ptrs {
template <typename T>
class gc_ptr_access;
}}

template <typename T>
class gc_ptr;

template<typename T>
class gc_pin: private details::ptrs::gc_untyped_pin
{
public:
    explicit gc_pin(const gc_ptr<T>& ptr);

    T* get() const;

    T& operator*() const;
    T* operator->();
    const T* operator->() const;
};

template <typename T>
class gc_pin<T[]> : private details::ptrs::gc_untyped_pin
{
public:
    explicit gc_pin(const gc_ptr<T[]>& ptr);

    T* get() const;
    T& operator*() const;
    T& operator[](size_t n) const;
};

template <typename T, size_t N>
class gc_pin<T[N]>;

template <typename T>
class gc_ptr: private details::ptrs::gc_untyped_ptr
{
    typedef details::ptrs::gc_untyped_ptr gc_untyped_ptr;
public:
    gc_ptr() {}

    gc_ptr(nullptr_t)
        : gc_untyped_ptr(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
        : gc_untyped_ptr(other)
    {}

    gc_ptr(gc_ptr&& other)
        : gc_untyped_ptr(std::move(other))
    {}

    gc_ptr& operator=(nullptr_t)
    {
        gc_untyped_ptr::operator=(nullptr);
        return *this;
    }

    gc_ptr& operator=(const gc_ptr& other)
    {
        gc_untyped_ptr::operator=(other);
        return *this;
    }

    gc_ptr& operator=(gc_ptr&& other)
    {
        gc_untyped_ptr::operator=(std::move(other));
        return *this;
    }

    void swap(gc_ptr& other)
    {
        gc_untyped_ptr::swap(other);
    }

    explicit operator bool() const
    {
        return gc_untyped_ptr::operator bool();
    }

    void reset()
    {
        gc_untyped_ptr::operator=(nullptr);
    }

    gc_pin<T> operator->()
    {
        return gc_pin<T>(*this);
    }

    gc_pin<T> operator->() const
    {
        return gc_pin<T>(*this);
    }

    friend bool operator==(const gc_ptr& p1, const gc_ptr& p2)
    {
        details::gc_unsafe_scope gc_unsafe;
        return p1.m_ptr.load() == p2.m_ptr.load();
    }

    friend bool operator!=(const gc_ptr& p1, const gc_ptr& p2)
    {
        details::gc_unsafe_scope gc_unsafe;
        return p1.m_ptr.load() != p2.m_ptr.load();
    }

    friend class gc_pin<T>;
    friend class details::ptrs::gc_ptr_access<T>;
private:
    gc_ptr(T* ptr)
        : gc_untyped_ptr((void*) ptr)
    {}
};

template <typename T>
class gc_ptr<T[]> : public details::iterator_facade<gc_ptr<T[]>, std::random_access_iterator_tag, T>
                  , private details::ptrs::gc_untyped_ptr
{
    typedef details::ptrs::gc_untyped_ptr gc_untyped_ptr;
public:
    gc_ptr() {}

    gc_ptr(nullptr_t)
        : gc_untyped_ptr(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
        : gc_untyped_ptr(other)
    {}

    gc_ptr(gc_ptr&& other)
        : gc_untyped_ptr(std::move(other))
    {}

    gc_ptr& operator=(nullptr_t)
    {
        gc_untyped_ptr::operator=(nullptr);
        return *this;
    }

    gc_ptr& operator=(const gc_ptr& other)
    {
        gc_untyped_ptr::operator=(other);
        return *this;
    }

    gc_ptr& operator=(gc_ptr&& other)
    {
        gc_untyped_ptr::operator=(std::move(other));
        return *this;
    }

    void swap(gc_ptr& other)
    {
        gc_untyped_ptr::swap(other);
    }

    explicit operator bool() const
    {
        return gc_untyped_ptr::operator bool;
    }

    void reset()
    {
        gc_untyped_ptr::operator=(nullptr);
    }

    friend class gc_pin<T[]>;
    friend class details::ptrs::gc_ptr_access<T[]>;
    friend class details::iterator_access<gc_ptr<T[]>>;
private:
    gc_ptr(T* ptr)
        : gc_untyped_ptr((void*) ptr)
    {}

    void increment()
    {
        m_ptr.fetch_add(sizeof(T));
    }

    void decrement()
    {
        m_ptr.fetch_sub(sizeof(T));
    }

    void advance(ptrdiff_t n)
    {
        m_ptr.fetch_add(n * sizeof(T));
    }

    ptrdiff_t difference(const gc_ptr& other) const
    {
        details::gc_unsafe_scope gc_unsafe;
        return (m_ptr.load() - other.m_ptr.load()) / sizeof(T);
    }

    bool equal(const gc_ptr& other) const
    {
        details::gc_unsafe_scope gc_unsafe;
        return m_ptr.load() == other.m_ptr.load();
    }

    bool less_than(const gc_ptr& other) const
    {
        details::gc_unsafe_scope gc_unsafe;
        return m_ptr.load() < other.m_ptr.load();
    }
};

template <typename T, size_t N>
class gc_ptr<T[N]>;

template <typename T>
gc_pin<T>::gc_pin(const gc_ptr<T>& ptr)
    : gc_untyped_pin(ptr)
{}

template <typename T>
T* gc_pin<T>::get() const
{
    return reinterpret_cast<T*>(details::ptrs::gc_untyped_pin::get());
}

template <typename T>
T& gc_pin<T>::operator*() const
{
    return *get();
}

template <typename T>
T* gc_pin<T>::operator->()
{
    return get();
}

template <typename T>
const T* gc_pin<T>::operator->() const
{
    return get();
}

template <typename T>
gc_pin<T[]>::gc_pin(const gc_ptr<T[]>& ptr)
        : gc_untyped_pin(ptr)
{}

template <typename T>
T* gc_pin<T[]>::get() const
{
    return reinterpret_cast<T*>(details::ptrs::gc_untyped_pin::get());
}

template <typename T>
T& gc_pin<T[]>::operator*() const
{
    return *(reinterpret_cast<T*>(details::ptrs::gc_untyped_pin::get()));
}

template <typename T>
T& gc_pin<T[]>::operator[](size_t n) const
{
    T* b = reinterpret_cast<T*>(details::ptrs::gc_untyped_pin::get());
    return *(b + n);
}

}

#endif //DIPLOMA_GC_PTR_H