#ifndef DIPLOMA_GC_PTR_H
#define DIPLOMA_GC_PTR_H

#include <cstdint>
#include <utility>
#include <iterator>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/gc_pin.hpp>
#include <libprecisegc/gc_ref.hpp>

namespace precisegc {

namespace details { namespace ptrs {
template <typename T>
class gc_ptr_access;
}}

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

    template <typename D, typename = typename std::enable_if<std::is_base_of<T, D>::value>::type>
    gc_ptr(const gc_ptr<D>& other)
        : gc_untyped_ptr(other)
    {};

    gc_ptr(gc_ptr&& other)
        : gc_untyped_ptr(std::move(other))
    {}

    template <typename D, typename = typename std::enable_if<std::is_base_of<T, D>::value>::type>
    gc_ptr(gc_ptr<D>&& other)
        : gc_untyped_ptr(std::move(other))
    {};

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

    void reset()
    {
        gc_untyped_ptr::operator=(nullptr);
    }

    gc_pin<T> pin() const
    {
        return gc_pin<T>(untyped_pin());
    }

    gc_ref<T> operator*() const
    {
        return gc_ref<T>(pin());
    }

    gc_pin<T> operator->() const
    {
        return pin();
    }

    explicit operator bool() const
    {
        return !is_null();
    }

    friend bool operator==(const gc_ptr& p1, const gc_ptr& p2)
    {
        return p1.equal(p2);
    }

    friend bool operator!=(const gc_ptr& p1, const gc_ptr& p2)
    {
        return !p1.equal(p2);
    }

    void swap(gc_ptr& other)
    {
        gc_untyped_ptr::swap(other);
    }

    friend void swap(gc_ptr& a, gc_ptr& b)
    {
        a.swap(b);
    }

    friend class details::ptrs::gc_ptr_access<T>;
private:
    gc_ptr(T* ptr)
        : gc_untyped_ptr((void*) ptr)
    {}
};

template <typename T>
class gc_ptr<T[]> : private details::ptrs::gc_untyped_ptr
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

    gc_ptr& operator+=(size_t n)
    {
        advance(n * sizeof(T));
        return *this;
    }

    gc_ptr& operator-=(size_t n)
    {
        advance(-n * sizeof(T));
        return *this;
    }

    void reset()
    {
        gc_untyped_ptr::operator=(nullptr);
    }

    gc_pin<T[]> pin() const
    {
        return gc_pin<T[]>(untyped_pin());
    }

    gc_ref<T[]> operator*() const
    {
        return gc_ref<T[]>(untyped_pin(), 0);
    }

    gc_ref<T[]> operator[](size_t n) const
    {
        return gc_ref<T[]>(untyped_pin(), n);
    }

    gc_ptr& operator++()
    {
        advance(sizeof(T));
        return *this;
    }

    gc_ptr& operator--()
    {
        advance(-sizeof(T));
        return *this;
    }

    gc_ptr operator++(int)
    {
        gc_ptr res = *this;
        advance(sizeof(T));
        return res;
    }

    gc_ptr operator--(int)
    {
        gc_ptr res = *this;
        advance(-sizeof(T));
        return res;
    }

    explicit operator bool() const
    {
        return !is_null();
    }

    friend bool operator==(const gc_ptr& p1, const gc_ptr& p2)
    {
        return p1.equal(p2);
    }

    friend bool operator!=(const gc_ptr& p1, const gc_ptr& p2)
    {
        return !p1.equal(p2);
    }

    friend gc_ptr operator+(const gc_ptr& p, size_t n)
    {
        return gc_ptr(p) += n;
    }

    friend gc_ptr operator+(size_t n, const gc_ptr& p)
    {
        return gc_ptr(p) += n;
    }

    friend gc_ptr operator-(const gc_ptr& p, size_t n)
    {
        return gc_ptr(p) -= n;
    }

    friend gc_ptr operator-(size_t n, const gc_ptr& p)
    {
        return gc_ptr(p) -= n;
    }

    void swap(gc_ptr& other)
    {
        gc_untyped_ptr::swap(other);
    }

    friend void swap(gc_ptr& a, gc_ptr& b)
    {
        a.swap(b);
    }

    friend class details::ptrs::gc_ptr_access<T[]>;
private:
    gc_ptr(T* ptr)
        : gc_untyped_ptr((void*) ptr)
    {}
};

template <typename T, size_t N>
class gc_ptr<T[N]>;

}

#endif //DIPLOMA_GC_PTR_H