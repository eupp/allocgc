#ifndef DIPLOMA_BLOCK_PTR_HPP
#define DIPLOMA_BLOCK_PTR_HPP

#include <cstddef>

namespace precisegc { namespace details { namespace utils {

template <typename Ptr>
class block_ptr
{
public:
    block_ptr()
        : m_size(0)
    {}

    explicit block_ptr(const Ptr& ptr, size_t size = 0)
        : m_ptr(ptr)
        , m_size(size)
    {}

    explicit block_ptr(Ptr&& ptr, size_t size = 0)
        : m_ptr(ptr)
        , m_size(size)
    {}

    block_ptr(const block_ptr&) = default;
    block_ptr(block_ptr&&) = default;

    block_ptr& operator=(const block_ptr&) = default;
    block_ptr& operator=(block_ptr&&) = default;

    size_t size() const
    {
        return m_size;
    }

    void set_size(size_t size)
    {
        m_size = size;
    }

    Ptr& decorated()
    {
        return m_ptr;
    }

    const Ptr& decorated() const
    {
        return m_ptr;
    }
private:
    Ptr m_ptr;
    size_t m_size;
};

template <typename Ptr>
block_ptr<Ptr> make_block_ptr(const Ptr& ptr, size_t size)
{
    return block_ptr<Ptr>(ptr, size);
}

}}}

#endif //DIPLOMA_BLOCK_PTR_HPP
