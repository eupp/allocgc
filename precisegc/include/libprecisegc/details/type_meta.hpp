#ifndef DIPLOMA_CLASS_META_H
#define DIPLOMA_CLASS_META_H

#include <cassert>
#include <memory>
#include <atomic>
#include <algorithm>
#include <type_traits>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/utils/dynarray.hpp>
#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

namespace internals {
class meta_holder;
}

class type_meta : private utils::noncopyable, private utils::nonmovable
{
    typedef utils::dynarray<size_t> dynarray_t;
public:
    typedef dynarray_t::const_iterator offsets_iterator;
    typedef boost::iterator_range<offsets_iterator> offsets_range;

    size_t type_size() const noexcept
    {
        return m_type_size;
    }

    bool is_plain_type() const noexcept
    {
        return m_offsets.empty();
    }

    offsets_range offsets() const
    {
        return boost::make_iterator_range(m_offsets.begin(), m_offsets.end());
    }
private:
    friend class internals::meta_holder;

    template <typename Iter>
    type_meta(size_t type_size, Iter offsets_first, Iter offsets_last)
        : m_offsets(offsets_first, offsets_last)
        , m_type_size(type_size)
    {}

    dynarray_t m_offsets;
    size_t m_type_size;
};

namespace internals {
class meta_holder : private utils::noncopyable, private utils::nonmovable
{
public:
    meta_holder()
        : m_meta_ptr(nullptr)
    {}

    ~meta_holder()
    {
        delete m_meta_ptr.load(std::memory_order_acquire);
    }

    bool is_initialized() const
    {
        return m_meta_ptr.load(std::memory_order_acquire) != nullptr;
    }

    template <typename Iter>
    void initialize(size_t type_size, Iter offsets_first, Iter offsets_last)
    {
        if (is_initialized()) {
            return;
        }
        std::unique_ptr<type_meta> meta_ptr(new type_meta(type_size, offsets_first, offsets_last));
        type_meta* expected = nullptr;
        if (m_meta_ptr.compare_exchange_strong(expected, meta_ptr.get(), std::memory_order_acq_rel)) {
            meta_ptr.release();
        }
    }

    type_meta* get() const
    {
        return m_meta_ptr.load(std::memory_order_acquire);
    }
private:
    std::atomic<type_meta*> m_meta_ptr;
};
}

template <typename T>
class type_meta_provider
{
public:
    static bool is_meta_created()
    {
        return meta.is_initialized();
    }

    static const type_meta* create_meta()
    {
        utils::dynarray<size_t> empty;
        meta.initialize(sizeof(T), empty.begin(), empty.end());
        return meta.get();
    }

    template <typename Range>
    static const type_meta* create_meta(Range&& range)
    {
        static_assert(std::is_same<typename Range::value_type, size_t>::value, "Offsets should have size_t type");
        meta.initialize(sizeof(T), range.begin(), range.end());
        return meta.get();
    }

    template <typename Iter>
    static const type_meta* create_meta(Iter offsets_begin, Iter offsets_end)
    {
        static_assert(std::is_same<typename Iter::value_type, size_t>::value, "Offsets should have size_t type");
        meta.initialize(sizeof(T), offsets_begin, offsets_end);
        return meta.get();
    }

    static const type_meta* get_meta()
    {
        assert(meta.get());
        return meta.get();
    }
private:
    static internals::meta_holder meta;
};

template <typename T>
internals::meta_holder type_meta_provider<T>::meta{};

} }

#endif //DIPLOMA_CLASS_META_H
