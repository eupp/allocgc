#ifndef GC_NEW_HPP
#define GC_NEW_HPP

#include <cstdio>
#include <cassert>
#include <type_traits>
#include <utility>

#include <liballocgc/gc_ptr.hpp>
#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/gc_type_meta.hpp>
#include <liballocgc/details/gc_facade.hpp>
#include <liballocgc/details/collectors/gc_new_stack_entry.hpp>
#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace pointers {

namespace internals {

template<typename T, typename GCStrategy>
struct gc_new_if
{
    typedef gc_ptr<T, GCStrategy> single_object;
};

template<typename T, typename GCStrategy>
struct gc_new_if<T[], GCStrategy>
{
    typedef gc_ptr<T[], GCStrategy> unknown_bound;
};

template<typename T, size_t N, typename GCStrategy>
struct gc_new_if<T[N], GCStrategy>
{
    typedef void known_bound;
};

template <typename GCStrategy>
class commiter : private details::utils::noncopyable, private details::utils::nonmovable
{
    typedef details::gc_facade<GCStrategy> gc_facade;
public:
    commiter(const gc_alloc::response* rsp)
        : m_rsp(rsp)
        , m_commited(false)
    {}

    ~commiter()
    {
        if (!m_commited) {
            gc_facade::abort(*m_rsp);
        }
    }

    void commit()
    {
        m_commited = true;
        gc_facade::commit(*m_rsp);
    }

    void commit(const gc_type_meta* tmeta)
    {
        m_commited = true;
        gc_facade::commit(*m_rsp, tmeta);
    }
private:
    const gc_alloc::response* m_rsp;
    bool m_commited;
};

}

template <typename T, typename GCStrategy, typename... Args>
auto gc_new(Args&&... args)
    -> typename internals::gc_new_if<T, GCStrategy>::single_object;

template <typename T, typename GCStrategy>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T, GCStrategy>::unknown_bound;


namespace internals {

template <typename T, typename GCStrategy>
class gc_ptr_factory
{
public:
    template <typename... Args>
    class instance
    {
    private:
        static gc_ptr<T, GCStrategy> create(T* ptr)
        {
            return create_internal(ptr);
        }

        friend gc_ptr<T, GCStrategy> gc_new<T, GCStrategy>(Args&&...);
    };
private:
    static gc_ptr<T, GCStrategy> create_internal(T* ptr)
    {
        return gc_ptr<T, GCStrategy>(ptr);
    }
};

template <typename T, typename GCStrategy>
class gc_ptr_factory<T[], GCStrategy>
{
    typedef typename std::remove_extent<T>::type U;
    typedef typename std::remove_cv<U>::type Uu;
public:
    static gc_ptr<T[], GCStrategy> create(Uu* ptr)
    {
        return gc_ptr<T[], GCStrategy>(ptr);
    }

    friend gc_ptr<T[], GCStrategy> gc_new<T[], GCStrategy>(size_t);
};

}

template <typename T, typename GCStrategy, typename... Args>
auto gc_new(Args&&... args)
    -> typename internals::gc_new_if<T, GCStrategy>::single_object
{
    typedef details::gc_facade<GCStrategy> gc_facade;
    typedef typename std::remove_cv<T>::type Tt;

    using namespace allocgc::details;

    gc_unsafe_scope unsafe_scope;

    const gc_type_meta* type_meta = gc_type_meta_factory<Tt>::get();

    gc_buf buffer;
    gc_alloc::response rsp = gc_facade::allocate(gc_alloc::request(sizeof(Tt), 1, type_meta, &buffer));
    internals::commiter<GCStrategy> commiter(&rsp);

    if (!type_meta) {
        new (rsp.obj_start()) Tt(std::forward<Args>(args)...);
        commiter.commit(gc_type_meta_factory<Tt>::create(gc_facade::make_offsets(rsp)));
    } else {
        new (rsp.obj_start()) Tt(std::forward<Args>(args)...);
        commiter.commit();
    }

    return internals::gc_ptr_factory<T, GCStrategy>::template instance<Args...>::create(
            reinterpret_cast<T*>(rsp.obj_start())
    );
};

template <typename T, typename GCStrategy>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T, GCStrategy>::unknown_bound
{
    typedef details::gc_facade<GCStrategy> gc_facade;
    typedef typename std::remove_extent<T>::type U;
    typedef typename std::remove_cv<U>::type Uu;

    using namespace allocgc::details;
    using namespace allocgc::details::threads;
    using namespace allocgc::details::allocators;

    gc_unsafe_scope unsafe_scope;

    const gc_type_meta* type_meta = gc_type_meta_factory<Uu>::get();

    gc_buf buffer;
    gc_alloc::response rsp = gc_facade::allocate(gc_alloc::request(sizeof(Uu), n, type_meta, &buffer));
    internals::commiter<GCStrategy> commiter(&rsp);

    bool type_meta_requested = (type_meta == nullptr);

    Uu* begin = reinterpret_cast<Uu*>(rsp.obj_start());
    Uu* end = begin + n;

    if (!type_meta) {
        new (begin++) Uu();
        type_meta = gc_type_meta_factory<Uu>::create(gc_facade::make_offsets(rsp));
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

    return internals::gc_ptr_factory<U[], GCStrategy>::create(
            reinterpret_cast<Uu*>(rsp.obj_start())
    );
};

template<typename T, typename GCStrategy, typename... Args>
auto gc_new(Args&&...)
    -> typename internals::gc_new_if<T, GCStrategy>::known_bound = delete;

}}

#endif //GC_NEW_HPP