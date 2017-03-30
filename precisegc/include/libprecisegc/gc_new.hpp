#ifndef GC_NEW_HPP
#define GC_NEW_HPP

#include <cstdio>
#include <cassert>
#include <type_traits>
#include <utility>

#include <libprecisegc/gc_ptr.hpp>
#include <libprecisegc/gc_alloc.hpp>
#include <libprecisegc/gc_type_meta.hpp>
#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/details/gc_hooks.hpp>

namespace precisegc {

namespace internals {

template<typename T>
struct gc_new_if
{
    typedef gc_ptr<T> single_object;
};

template<typename T>
struct gc_new_if<T[]>
{
    typedef gc_ptr<T[]> unknown_bound;
};

template<typename T, size_t N>
struct gc_new_if<T[N]>
{
    typedef void known_bound;
};

class commiter : private details::utils::noncopyable, private details::utils::nonmovable
{
public:
    commiter(const gc_alloc::response* rsp)
        : m_rsp(rsp)
        , m_commited(false)
    {}

    ~commiter()
    {
        if (!m_commited) {
            details::gc_abort(*m_rsp);
        }
    }

    void commit()
    {
        m_commited = true;
        details::gc_commit(*m_rsp);
    }

    void commit(const gc_type_meta* tmeta)
    {
        m_commited = true;
        details::gc_commit(*m_rsp, tmeta);
    }
private:
    const gc_alloc::response* m_rsp;
    bool m_commited;
};

}

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> typename internals::gc_new_if<T>::single_object;

template <typename T>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T>::unknown_bound;


namespace internals {

template <typename T>
class gc_ptr_factory
{
public:
    template <typename... Args>
    class instance
    {
    private:
        static gc_ptr<T> create(T* ptr)
        {
            return create_internal(ptr);
        }

        friend gc_ptr<T> gc_new<T>(Args&&...);
    };
private:
    static gc_ptr<T> create_internal(T* ptr)
    {
        return gc_ptr<T>(ptr);
    }
};

template <typename T>
class gc_ptr_factory<T[]>
{
    typedef typename std::remove_extent<T>::type U;
    typedef typename std::remove_cv<U>::type Uu;
public:
    static gc_ptr<T[]> create(Uu* ptr)
    {
        return gc_ptr<T[]>(ptr);
    }

    friend gc_ptr<T[]> gc_new<T[]>(size_t);
};

}

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> typename internals::gc_new_if<T>::single_object
{
    using namespace precisegc::details;

    gc_unsafe_scope unsafe_scope;

    typedef typename std::remove_cv<T>::type Tt;

    const gc_type_meta* type_meta = gc_type_meta_factory<Tt>::get();

    gc_buf buffer;
    gc_alloc::response rsp = gc_allocate(gc_alloc::request(sizeof(Tt), 1, type_meta, &buffer));
    internals::commiter commiter(&rsp);

    if (!type_meta) {
        new (rsp.obj_start()) Tt(std::forward<Args>(args)...);
        commiter.commit(gc_type_meta_factory<Tt>::create(gc_make_offsets(rsp)));
    } else {
        new (rsp.obj_start()) Tt(std::forward<Args>(args)...);
        commiter.commit();
    }

    return precisegc::internals::gc_ptr_factory<T>::template instance<Args...>::create(
            reinterpret_cast<T*>(rsp.obj_start())
    );
};

template <typename T>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T>::unknown_bound
{
    using namespace precisegc::details;
    using namespace precisegc::details::threads;
    using namespace precisegc::details::allocators;

    typedef typename std::remove_extent<T>::type U;
    typedef typename std::remove_cv<U>::type Uu;

    gc_unsafe_scope unsafe_scope;

    const gc_type_meta* type_meta = gc_type_meta_factory<Uu>::get();

    gc_buf buffer;
    gc_alloc::response rsp = gc_allocate(gc_alloc::request(sizeof(Uu), n, type_meta, &buffer));
    internals::commiter commiter(&rsp);

    bool type_meta_requested = (type_meta == nullptr);

    Uu* begin = reinterpret_cast<Uu*>(rsp.obj_start());
    Uu* end = begin + n;

    if (!type_meta) {
        new (begin++) Uu();
        type_meta = gc_type_meta_factory<Uu>::create(gc_make_offsets(rsp));
    } else {
        new (begin++) Uu();
    }

    for (Uu* it = begin; it < end; ++it) {
        new (it) Uu();
    }

    if (type_meta_requested) {
        commiter.commit(type_meta);
    } else {
        commiter.commit();
    }

    return precisegc::internals::gc_ptr_factory<U[]>::create(
            reinterpret_cast<Uu*>(rsp.obj_start())
    );
};

template<typename T, typename... Args>
auto gc_new(Args&&...)
        -> typename internals::gc_new_if<T>::known_bound
        = delete;

}

#endif //GC_NEW_HPP