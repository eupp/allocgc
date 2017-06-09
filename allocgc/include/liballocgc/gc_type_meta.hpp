#ifndef ALLOCGC_GC_TYPE_META_HPP
#define ALLOCGC_GC_TYPE_META_HPP

#include <vector>
#include <type_traits>

#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/utils/dynarray.hpp>
#include <liballocgc/details/utils/to_string.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/gc_exception.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc {

class forbidden_move_exception : public gc_exception
{
public:
    forbidden_move_exception(byte* ptr)
        : gc_exception("Attempt to move non-movable object by address " + details::utils::to_string(ptr))
    {}
};

class gc_type_meta : private details::utils::noncopyable, private details::utils::nonmovable
{
    // unknown bug with dynarray --> use vector instead
    // typedef utils::dynarray<size_t> offset_container_t;
    typedef std::vector<size_t> offset_container_t;
    typedef offset_container_t::const_iterator offsets_iterator_t;
public:
    typedef boost::iterator_range<offsets_iterator_t> offsets_t;

    inline size_t type_size() const noexcept
    {
        return m_type_size;
    }

    inline bool is_plain_type() const noexcept
    {
        return m_offsets.empty();
    }

    inline offsets_t offsets() const
    {
        return boost::make_iterator_range(m_offsets.cbegin(), m_offsets.cend());
    }

    inline bool is_movable() const noexcept
    {
        return m_is_movable;
    }

    virtual bool is_trivially_destructible() const = 0;
    virtual void destroy(byte* ptr) const = 0;
    virtual void move(byte* from, byte* to) const = 0;
protected:
    template <typename Iter>
    gc_type_meta(size_t type_size,
                 bool is_movable,
                 Iter offsets_first,
                 Iter offsets_last)
        : m_offsets(offsets_first, offsets_last)
        , m_type_size(type_size)
        , m_is_movable(is_movable)
    {}
private:
    offset_container_t m_offsets;
    size_t m_type_size;
    bool m_is_movable;
};

template <typename T>
class gc_type_meta_factory;

namespace internals {

template <typename T>
class gc_type_meta_instance : public gc_type_meta
{
public:
    bool is_trivially_destructible() const override
    {
        return std::is_trivially_destructible<T>::value;
    }

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
class gc_type_meta_factory : private details::utils::nonconstructible
{
public:
    static const gc_type_meta* get()
    {
        return meta.load(std::memory_order_acquire);
    }

    static const gc_type_meta* create()
    {
        details::utils::dynarray<size_t> empty;
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


}

#endif //ALLOCGC_GC_TYPE_META_HPP
