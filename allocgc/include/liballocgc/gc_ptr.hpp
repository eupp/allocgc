#ifndef ALLOCGC_GC_PTR_H
#define ALLOCGC_GC_PTR_H

#include <cstdint>
#include <utility>
#include <iterator>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/gc_pin.hpp>
#include <liballocgc/gc_ref.hpp>
#include <liballocgc/details/gc_facade.hpp>
#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/details/gc_untyped_ptr.hpp>
#include <liballocgc/details/utils/base_offset.hpp>

namespace allocgc { namespace pointers {

namespace internals {

template <typename T, typename GCStrategy>
class gc_ptr_factory;

// for testing purpose
class gc_ptr_access;

}

template <typename T, typename GCStrategy>
class gc_ptr: private gc_untyped_ptr<GCStrategy>
{
    typedef gc_untyped_ptr<GCStrategy> base;
public:
    gc_ptr() {}

    gc_ptr(std::nullptr_t)
        : base(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
        : base(other)
    {}

    template <typename D, typename = typename std::enable_if<std::is_base_of<T, D>::value>::type>
    gc_ptr(const gc_ptr<D, GCStrategy>& other)
        : base(other)
    {
        shift_to_base<D>();
    };

    gc_ptr& operator=(std::nullptr_t)
    {
        base::operator=(nullptr);
        return *this;
    }

    gc_ptr& operator=(const gc_ptr& other)
    {
        base::operator=(other);
        return *this;
    }

    template <typename D, typename = typename std::enable_if<std::is_base_of<T, D>::value>::type>
    gc_ptr& operator=(const gc_ptr<D, GCStrategy>& other)
    {
        base::operator=(other);
        shift_to_base<D>();
        return *this;
    };

    void reset()
    {
        base::operator=(nullptr);
    }

    bool is_root() const
    {
        return base::is_root();
    }

    gc_pin<T, GCStrategy> pin() const
    {
        return gc_pin<T, GCStrategy>(base::untyped_pin());
    }

    gc_ref<T, GCStrategy> operator*() const
    {
        return gc_ref<T, GCStrategy>(pin());
    }

    gc_stack_pin<T, GCStrategy> operator->() const
    {
        return gc_stack_pin<T, GCStrategy>(base::push_untyped_pin());
    }

    explicit operator bool() const
    {
        return !base::is_null();
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
        base::swap(other);
    }

    friend void swap(gc_ptr& a, gc_ptr& b)
    {
        a.swap(b);
    }

    template <typename U, typename GCStrategy1>
    friend class gc_ptr;

//    template <typename U, typename V>
//    friend gc_ptr<U> static_pointer_cast(const gc_ptr<V>&);
//
//    template <typename U, typename V>
//    friend gc_ptr<U> dynamic_pointer_cast(const gc_ptr<V>&);
//
//    template <typename U, typename V>
//    friend gc_ptr<U> const_pointer_cast(const gc_ptr<V>&);
//
//    template <typename U, typename V>
//    friend gc_ptr<U> reinterpret_pointer_cast(const gc_ptr<V>&);
//
//    template <typename Tt, typename R, R Tt::*member>
//    friend gc_ptr<R> take_interior(const gc_ptr<Tt>&);

    friend class internals::gc_ptr_factory<T, GCStrategy>;
    friend class internals::gc_ptr_access;
private:
    explicit gc_ptr(T* ptr)
        : base(reinterpret_cast<byte*>(ptr))
    {}

    T* get() const
    {
        return reinterpret_cast<T*>(base::rbarrier());
    }

    template <typename D>
    void shift_to_base()
    {
        static const ptrdiff_t offset = details::utils::base_offset<T>(reinterpret_cast<D*>(base::rbarrier()));
        base::advance(offset);
    }
};

template <typename T, typename GCStrategy>
class gc_ptr<T[], GCStrategy> : private gc_untyped_ptr<GCStrategy>
{
    typedef gc_untyped_ptr<GCStrategy> base;
public:
    gc_ptr() {}

    gc_ptr(std::nullptr_t)
        : base(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
        : base(other)
    {}

    gc_ptr(gc_ptr&& other)
        : base(std::move(other))
    {}

    gc_ptr& operator=(std::nullptr_t)
    {
        base::operator=(nullptr);
        return *this;
    }

    gc_ptr& operator=(const gc_ptr& other)
    {
        base::operator=(other);
        return *this;
    }

    gc_ptr& operator=(gc_ptr&& other)
    {
        base::operator=(std::move(other));
        return *this;
    }

    gc_ptr& operator+=(size_t n)
    {
        base::advance(n * sizeof(T));
        return *this;
    }

    gc_ptr& operator-=(size_t n)
    {
        base::advance(-n * sizeof(T));
        return *this;
    }

    void reset()
    {
        base::operator=(nullptr);
    }

    bool is_root() const
    {
        return base::is_root();
    }

    gc_pin<T[], GCStrategy> pin() const
    {
        return gc_pin<T[], GCStrategy>(base::untyped_pin());
    }

    gc_ref<T[], GCStrategy> operator[](size_t n) const
    {
        return gc_ref<T[], GCStrategy>(base::untyped_pin(), n);
    }

    gc_ptr& operator++()
    {
        base::advance(sizeof(T));
        return *this;
    }

    gc_ptr& operator--()
    {
        base::advance(-sizeof(T));
        return *this;
    }

    gc_ptr operator++(int)
    {
        gc_ptr res = *this;
        base::advance(sizeof(T));
        return res;
    }

    gc_ptr operator--(int)
    {
        gc_ptr res = *this;
        base::advance(-sizeof(T));
        return res;
    }

    explicit operator bool() const
    {
        return !base::is_null();
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
        base::swap(other);
    }

    friend void swap(gc_ptr& a, gc_ptr& b)
    {
        a.swap(b);
    }

//    template <typename U, typename V>
//    friend gc_ptr<U[]> const_pointer_cast(const gc_ptr<V[]>&);
//
//    template <typename U, typename V>
//    friend gc_ptr<U> reinterpret_pointer_cast(const gc_ptr<V>&);

    friend class internals::gc_ptr_factory<T[], GCStrategy>;
    friend class internals::gc_ptr_access;
private:
    typedef typename std::remove_cv<T>::type Tt;

    explicit gc_ptr(T* ptr)
        : base(reinterpret_cast<byte*>(const_cast<Tt*>(ptr)))
    {}

    T* get() const
    {
        return reinterpret_cast<T*>(base::rbarrier());
    }
};

template <typename T, size_t N, typename GCStrategy>
class gc_ptr<T[N], GCStrategy>;

namespace internals {

class gc_ptr_access
{
public:
    template <typename T, typename GCStrategy>
    static gc_untyped_ptr<GCStrategy>& get_untyped(gc_ptr<T, GCStrategy>& ptr)
    {
        return (gc_untyped_ptr<GCStrategy>&) ptr;
    }

    template <typename T, typename GCStrategy>
    static typename std::remove_extent<T>::type* get(const gc_ptr<T, GCStrategy>& ptr)
    {
        typedef typename std::remove_extent<T>::type U;
        return reinterpret_cast<U*>(ptr.get());
    }
};

}

}}

#endif //ALLOCGC_GC_PTR_H