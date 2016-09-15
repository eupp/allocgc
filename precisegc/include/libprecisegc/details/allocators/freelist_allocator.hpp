#ifndef DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP
#define DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/utils/get_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

struct fixsize_policy {};
struct varsize_policy {};

template <typename SizePolicy>
using is_varsize_policy = std::is_same<SizePolicy, varsize_policy>;

template <typename UpstreamAlloc, typename SizePolicy>
class freelist_allocator : private utils::ebo<UpstreamAlloc>,
                                   private utils::noncopyable, private utils::nonmovable
{
public:
    typedef typename UpstreamAlloc::pointer_type pointer_type;
    typedef typename UpstreamAlloc::memory_range_type memory_range_type;
    typedef stateful_alloc_tag alloc_tag;
private:
    typedef typename std::conditional<
              is_varsize_policy<SizePolicy>::value
            , utils::block_ptr<pointer_type>
            , pointer_type
        >::type internal_pointer_type;
public:
    freelist_allocator()
        : m_head(nullptr)
    {}

    pointer_type allocate(size_t size)
    {
        assert(sizeof(internal_pointer_type) <= size);
        if (m_head) {
            internal_pointer_type ptr = m_head;
            m_head = *reinterpret_cast<internal_pointer_type*>(utils::get_ptr(ptr));
            return from_internal_pointer(ptr);
        }
        return mutable_upstream_allocator().allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        assert(ptr);
        assert(sizeof(pointer_type) <= size);
        internal_pointer_type* next = reinterpret_cast<internal_pointer_type*>(utils::get_ptr(ptr));
        *next = m_head;
        m_head = to_internal_pointer(ptr, size);
    }

    template <typename SP = SizePolicy>
    auto shrink()
        -> typename std::enable_if<is_varsize_policy<SP>::value, size_t>::type
    {
        auto prev = m_head;
        while (m_head) {
            internal_pointer_type next = *reinterpret_cast<internal_pointer_type*>(utils::get_ptr(m_head));
            mutable_upstream_allocator().deallocate(m_head.decorated(), m_head.size());
            prev = m_head;
            m_head = next;
        }
        return mutable_upstream_allocator().shrink();
    }

    template <typename SP = SizePolicy>
    auto shrink(size_t size)
        -> typename std::enable_if<!is_varsize_policy<SP>::value, size_t>::type
    {
        while (m_head) {
            internal_pointer_type next = *reinterpret_cast<internal_pointer_type*>(utils::get_ptr(m_head));
            mutable_upstream_allocator().deallocate(m_head, size);
            m_head = next;
        }
        return mutable_upstream_allocator().shrink();
    }

    bool empty() const
    {
        return !m_head && upstream_allocator().empty();
    }

    void reset()
    {
        m_head = nullptr;
    }

    void reset_cache()
    {
        mutable_upstream_allocator().reset_cache();
    }

    template <typename Functor>
    void apply_to_chunks(Functor&& f)
    {
        mutable_upstream_allocator().apply_to_chunks(std::forward<Functor>(f));
    }

    memory_range_type memory_range()
    {
        return mutable_upstream_allocator().memory_range();
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }
private:
    template <typename SP = SizePolicy>
    auto to_internal_pointer(const pointer_type& ptr, size_t size)
        -> typename std::enable_if<is_varsize_policy<SP>::value, internal_pointer_type>::type
    {
        return utils::make_block_ptr(ptr, size);
    }

    template <typename SP = SizePolicy>
    auto to_internal_pointer(const pointer_type& ptr, size_t size)
        -> typename std::enable_if<!is_varsize_policy<SP>::value, internal_pointer_type>::type
    {
        return ptr;
    }

    template <typename SP = SizePolicy>
    auto from_internal_pointer(const internal_pointer_type& ptr)
        -> typename std::enable_if<is_varsize_policy<SP>::value, internal_pointer_type>::type
    {
        return ptr.decorated();
    }

    template <typename SP = SizePolicy>
    auto from_internal_pointer(const internal_pointer_type& ptr)
        -> typename std::enable_if<!is_varsize_policy<SP>::value, internal_pointer_type>::type
    {
        return ptr;
    }

    UpstreamAlloc& mutable_upstream_allocator()
    {
        return this->template get_base<UpstreamAlloc>();
    }

    internal_pointer_type m_head;
};

}}}

#endif //DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP