#ifndef DIPLOMA_GC_TYPE_META_FACTORY_HPP
#define DIPLOMA_GC_TYPE_META_FACTORY_HPP

#include <atomic>
#include <memory>
#include <type_traits>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_word.hpp>
#include <libprecisegc/details/gc_type_meta.hpp>

namespace precisegc { namespace details {

template <typename T>
class gc_type_meta_factory;

namespace internals {

template <typename T>
class gc_type_meta_instance : public gc_type_meta
{
public:
    void destroy(byte* ptr, size_t obj_cnt) const override
    {
        size_t obj_size = type_size();
        for (size_t i = 0; i < obj_cnt; ++i, ptr += obj_size) {
            for (size_t offset: offsets()) {
                gc_word* word = reinterpret_cast<gc_word*>(ptr + offset);
                gc_handle_access::set<std::memory_order_relaxed>(*word, nullptr);
            }
            reinterpret_cast<T*>(ptr)->~T();
        }
    }

    void move(byte* from, byte* to, size_t obj_cnt) const override
    {
        size_t obj_size = type_size();
        for (size_t i = 0; i < obj_cnt; ++i, from += obj_size, to += obj_size) {
            move_impl(from, to);
        }
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
    static const gc_type_meta* get()
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

        const gc_type_meta* type_meta = get();
        if (type_meta != nullptr) {
            return type_meta;
        }

        std::unique_ptr<gc_type_meta> meta_owner(new internals::gc_type_meta_instance<T>(offsets_begin, offsets_end));
        if (meta.compare_exchange_strong(type_meta, meta_owner.get(), std::memory_order_acq_rel)) {
            meta_owner.release();
        }

        return meta.load(std::memory_order_relaxed);
    }
private:
    static std::atomic<const gc_type_meta*> meta;
};

template <typename T>
std::atomic<const gc_type_meta*> gc_type_meta_factory<T>::meta{nullptr};

}}

#endif //DIPLOMA_GC_TYPE_META_FACTORY_HPP
