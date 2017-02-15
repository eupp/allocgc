#ifndef DIPLOMA_GC_PTR_H
#define DIPLOMA_GC_PTR_H

#include <cstdint>
#include <utility>
#include <iterator>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/base_offset.hpp>
#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/gc_pin.hpp>
#include <libprecisegc/gc_ref.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>

namespace precisegc {

namespace internals {

template <typename T>
class gc_ptr_factory;

// for testing purpose
template <typename T>
class gc_ptr_access;

}

template <typename T>
class gc_ptr: private details::ptrs::gc_untyped_ptr
{
    typedef details::ptrs::gc_untyped_ptr gc_untyped_ptr;
public:
    gc_ptr() {}

    gc_ptr(std::nullptr_t)
        : gc_untyped_ptr(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
        : gc_untyped_ptr(other)
    {}

    template <typename D, typename = typename std::enable_if<std::is_base_of<T, D>::value>::type>
    gc_ptr(const gc_ptr<D>& other)
        : gc_untyped_ptr(other)
    {
        shift_to_base<D>();
    };

    gc_ptr& operator=(std::nullptr_t)
    {
        gc_untyped_ptr::operator=(nullptr);
        return *this;
    }

    gc_ptr& operator=(const gc_ptr& other)
    {
        gc_untyped_ptr::operator=(other);
        return *this;
    }

    template <typename D, typename = typename std::enable_if<std::is_base_of<T, D>::value>::type>
    gc_ptr& operator=(const gc_ptr<D>& other)
    {
        gc_untyped_ptr::operator=(other);
        shift_to_base<D>();
        return *this;
    };

    void reset()
    {
        gc_untyped_ptr::operator=(nullptr);
    }

    bool is_root() const
    {
        return gc_untyped_ptr::is_root();
    }

    gc_pin<T> pin() const
    {
        return gc_pin<T>(untyped_pin());
    }

    gc_ref<T> operator*() const
    {
        return gc_ref<T>(pin());
    }

    gc_stack_pin<T> operator->() const
    {
        return gc_stack_pin<T>(push_untyped_pin());
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

    template <typename U>
    friend class gc_ptr;

    template <typename U, typename V>
    friend gc_ptr<U> static_pointer_cast(const gc_ptr<V>&);

    template <typename U, typename V>
    friend gc_ptr<U> dynamic_pointer_cast(const gc_ptr<V>&);

    template <typename U, typename V>
    friend gc_ptr<U> const_pointer_cast(const gc_ptr<V>&);

    template <typename U, typename V>
    friend gc_ptr<U> reinterpret_pointer_cast(const gc_ptr<V>&);

    template <typename Tt, typename R, R Tt::*member>
    friend gc_ptr<R> take_interior(const gc_ptr<Tt>&);

    friend class internals::gc_ptr_factory<T>;
    friend class internals::gc_ptr_access<T>;
private:
    explicit gc_ptr(T* ptr)
        : gc_untyped_ptr(reinterpret_cast<byte*>(ptr))
    {}

    T* get() const
    {
        return reinterpret_cast<T*>(gc_untyped_ptr::get());
    }

    template <typename D>
    void shift_to_base()
    {
        static const ptrdiff_t offset = details::utils::base_offset<T>(reinterpret_cast<D*>(gc_untyped_ptr::get()));
        advance(offset);
    }
};

template <typename T>
class gc_ptr<T[]> : private details::ptrs::gc_untyped_ptr
{
    typedef details::ptrs::gc_untyped_ptr gc_untyped_ptr;
public:
    gc_ptr() {}

    gc_ptr(std::nullptr_t)
        : gc_untyped_ptr(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
        : gc_untyped_ptr(other)
    {}

    gc_ptr(gc_ptr&& other)
        : gc_untyped_ptr(std::move(other))
    {}

    gc_ptr& operator=(std::nullptr_t)
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

    bool is_root() const
    {
        return gc_untyped_ptr::is_root();
    }

    gc_pin<T[]> pin() const
    {
        return gc_pin<T[]>(untyped_pin());
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

    friend gc_ptr operator+(gc_ptr p, size_t n)
    {
        return p += n;
    }

    friend gc_ptr operator+(size_t n, gc_ptr p)
    {
        return p += n;
    }

    friend gc_ptr operator-(gc_ptr p, size_t n)
    {
        return p -= n;
    }

    friend gc_ptr operator-(size_t n, gc_ptr p)
    {
        return p -= n;
    }

    void swap(gc_ptr& other)
    {
        gc_untyped_ptr::swap(other);
    }

    friend void swap(gc_ptr& a, gc_ptr& b)
    {
        a.swap(b);
    }

    template <typename U, typename V>
    friend gc_ptr<U[]> const_pointer_cast(const gc_ptr<V[]>&);

    template <typename U, typename V>
    friend gc_ptr<U> reinterpret_pointer_cast(const gc_ptr<V>&);

    friend class internals::gc_ptr_factory<T[]>;
    friend class internals::gc_ptr_access<T[]>;
private:
    typedef typename std::remove_cv<T>::type Tt;

    explicit gc_ptr(T* ptr)
        : gc_untyped_ptr(reinterpret_cast<byte*>(const_cast<Tt*>(ptr)))
    {}

    T* get() const
    {
        return reinterpret_cast<T*>(gc_untyped_ptr::get());
    }
};

template <typename T, size_t N>
class gc_ptr<T[N]>;

namespace internals {

template <typename T>
class gc_ptr_access
{
    typedef typename std::remove_extent<T>::type U;
public:
    static details::ptrs::gc_untyped_ptr& get_untyped(gc_ptr<T>& ptr)
    {
        return (details::ptrs::gc_untyped_ptr&) ptr;
    }

    static U* get(const gc_ptr<T>& ptr)
    {
        return reinterpret_cast<U*>(ptr.get());
    }
};

}

}

#endif //DIPLOMA_GC_PTR_H