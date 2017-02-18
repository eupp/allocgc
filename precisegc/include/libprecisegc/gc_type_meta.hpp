#ifndef DIPLOMA_GC_TYPE_META_HPP
#define DIPLOMA_GC_TYPE_META_HPP

#include <vector>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/utils/dynarray.hpp>
#include <libprecisegc/details/utils/to_string.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/gc_exception.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc {

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

    virtual void destroy(byte* ptr) const = 0;
    virtual void move(byte* from, byte* to) const = 0;

    virtual bool with_destructor() const = 0;
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

}

#endif //DIPLOMA_GC_TYPE_META_HPP
