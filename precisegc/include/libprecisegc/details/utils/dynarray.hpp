#ifndef DIPLOMA_DYNARRAY_HPP
#define DIPLOMA_DYNARRAY_HPP

#include <cstddef>

#include <memory>
#include <iterator>
#include <algorithm>
#include <utility>

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/utils/make_unique.hpp>

namespace precisegc { namespace details { namespace utils {

template <typename T>
class dynarray
{
    template <typename Value>
    class iterator_base : public boost::iterator_facade<
              iterator_base<Value>
            , Value
            , boost::random_access_traversal_tag
            , Value&
            , ptrdiff_t
        >
    {
    public:
        iterator_base() noexcept
            : m_ptr(nullptr)
        {}

        iterator_base(const iterator_base&) noexcept = default;
        iterator_base(iterator_base&&) noexcept = default;

        iterator_base& operator=(const iterator_base&) noexcept = default;
        iterator_base& operator=(iterator_base&&) noexcept = default;
    private:
        friend class dynarray;
        friend class boost::iterator_core_access;

        iterator_base(Value* ptr)
            : m_ptr(ptr)
        {}

        Value& dereference() const noexcept
        {
            return *m_ptr;
        }

        void increment() noexcept
        {
            ++m_ptr;
        }

        void decrement() noexcept
        {
            --m_ptr;
        }

        bool equal(const iterator_base& other) const noexcept
        {
            return m_ptr == other.m_ptr;
        }

        void advance(ptrdiff_t n) noexcept
        {
            m_ptr += n;
        }

        ptrdiff_t distance_to(const iterator_base& other) const noexcept
        {
            return other.m_ptr - m_ptr;
        }

        Value* m_ptr;
    };
public:
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef iterator_base<value_type>       iterator;
    typedef iterator_base<const value_type> const_iterator;

    constexpr dynarray()
        : m_data(nullptr)
        , m_size(0)
    {}

    explicit dynarray(size_type size)
        : m_data(size > 0 ? utils::make_unique<T[]>(size) : nullptr)
        , m_size(size)
    {}

    dynarray(size_type size, const T& value)
        : dynarray(size)
    {
        std::fill(begin(), end(), value);
    }

    template <typename Iter>
    dynarray(Iter first, Iter last)
        : dynarray(std::distance(first, last))
    {
        std::copy(first, last, begin());
    }

    dynarray(std::initializer_list<T> init)
        : dynarray(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    dynarray(const dynarray& other)
        : dynarray(other.size())
    {
        std::copy(other.begin(), other.end(), begin());
    }

    dynarray(dynarray&& other)
        : m_data(std::move(other.m_data))
        , m_size(other.m_size)
    { }

    dynarray& operator=(dynarray other)
    {
        swap(other);
        return *this;
    }

    reference operator[](size_type n) noexcept
    {
        return *(m_data.get() + n);
    }

    const_reference operator[](size_type n) const noexcept
    {
        return *(m_data.get() + n);
    }

    reference front() noexcept
    {
        return *m_data.get();
    }

    const_reference front() const noexcept
    {
        return *m_data.get();
    }

    reference back() noexcept
    {
        return *(m_data.get() + m_size - 1);
    }

    const_reference back() const noexcept
    {
        return *(m_data.get() + m_size - 1);
    }

    T* data() noexcept
    {
        return m_data.get();
    }

    const T* data() const noexcept
    {
        return m_data.get();
    }

    bool empty() const noexcept
    {
        return m_data == nullptr;
    }

    size_type size() const noexcept
    {
        return m_size;
    }

    iterator begin()
    {
        return iterator(m_data.get());
    }

    const_iterator begin() const
    {
        return const_iterator(m_data.get());
    }

    const_iterator cbegin() const
    {
        return const_iterator(m_data.get());
    }

    iterator end()
    {
        return iterator(m_data.get() + m_size);
    }

    const_iterator end() const
    {
        return const_iterator(m_data.get() + m_size);
    }

    const_iterator cend() const
    {
        return const_iterator(m_data.get() + m_size);
    }

    void swap(dynarray& other)
    {
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_size, other.m_size);
    }

    friend void swap(dynarray& a, dynarray& b)
    {
        a.swap(b);
    }
private:
    std::unique_ptr<value_type[]> m_data;
    size_type m_size;
};

}}}

#endif //DIPLOMA_DYNARRAY_HPP
