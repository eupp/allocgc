#ifndef DIPLOMA_GC_TYPE_META_FACTORY_HPP
#define DIPLOMA_GC_TYPE_META_FACTORY_HPP

#include <atomic>
#include <memory>
#include <type_traits>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/utils/to_string.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/gc_type_meta.hpp>

namespace precisegc { namespace details {

template <typename T>
class gc_type_meta_factory;

namespace internals {

class forbidden_move_exception : public gc_exception
{
public:
    forbidden_move_exception(byte* ptr)
        : gc_exception("Attempt to move non-movable object by address " + utils::to_string(ptr))
    {}
};

template <typename T>
class gc_type_meta_instance : public gc_type_meta
{
public:
    void destroy(byte* ptr) const override
    {
        reinterpret_cast<T*>(ptr)->~T();
    }

    void move(byte* from, byte* to) const override
    {
        move_impl(from, to);
    }

    friend class gc_type_meta_factory<T>;
private:
    template <typename Iter>
    gc_type_meta_instance(Iter offsets_first, Iter offsets_last)
        : gc_type_meta(sizeof(T), std::is_move_constructible<T>::value, offsets_first, offsets_last)
    {}

    template <typename U = T>
    auto move_impl(byte* from, byte* to) const
        -> typename std::enable_if<std::is_move_constructible<U>::value>::type
    {
        new (to) T(std::move(*reinterpret_cast<T*>(from)));
    }

    template <typename U = T>
    auto move_impl(byte* from, byte* to) const
        -> typename std::enable_if<!std::is_move_constructible<U>::value>::type
    {
        throw forbidden_move_exception(from);
    }
};

}

template <typename T>
class gc_type_meta_factory : private utils::nonconstructible
{
public:
    static gc_type_meta* get()
    {
        return meta.load(std::memory_order_acquire);
    }

    static const gc_type_meta* create()
    {
        utils::dynarray<size_t> empty;
        return create(empty.begin(), empty.end());
    }

    template <typename Range>
    static const gc_type_meta* create(Range&& range)
    {
        return create(range.begin(), range.end());
    }

    template <typename Iter>
    static const gc_type_meta* create(Iter offsets_begin, Iter offsets_end)
    {
        static_assert(std::is_same<typename Iter::value_type, size_t>::value, "Offsets should have size_t type");

        gc_type_meta* tmeta = get();
        if (tmeta != nullptr) {
            return tmeta;
        }

        std::unique_ptr<gc_type_meta> meta_owner(new internals::gc_type_meta_instance<T>(offsets_begin, offsets_end));
        if (meta.compare_exchange_strong(tmeta, meta_owner.get(), std::memory_order_acq_rel)) {
            meta_owner.release();
        }

        return meta.load(std::memory_order_relaxed);
    }
private:
    static std::atomic<gc_type_meta*> meta;
};

template <typename T>
std::atomic<gc_type_meta*> gc_type_meta_factory<T>::meta{nullptr};

}}

#endif //DIPLOMA_GC_TYPE_META_FACTORY_HPP
