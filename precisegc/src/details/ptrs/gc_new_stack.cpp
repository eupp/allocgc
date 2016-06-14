#include "libprecisegc/details/ptrs/gc_new_stack.hpp"

#include <cassert>
#include <cstdint>

namespace precisegc { namespace details { namespace ptrs {

class stack_top
{
public:
    constexpr stack_top()
        : m_top_ptr(nullptr)
        , m_top_size(0)
        , m_depth(0)
        , m_is_meta_requested(false)
        , m_top_offsets_cnt(0)
    {}

    void register_child(gc_untyped_ptr* child)
    {
        std::uintptr_t top  = reinterpret_cast<std::uintptr_t>(m_top_ptr);
        std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(child);
        if (top <= curr && curr < top + m_top_size) {
            push_offset(curr - top);
        }
    }

    gc_new_stack::offsets_range offsets() const
    {
        return boost::make_iterator_range(m_top_offsets.begin(), std::next(m_top_offsets.begin(), m_top_offsets_cnt));
    }

    size_t depth() const noexcept
    {
        return m_depth;
    }

    bool is_active() const noexcept
    {
        return m_depth > 0;
    }

    bool is_meta_requsted() const noexcept
    {
        return m_is_meta_requested;
    }

    friend class gc_new_stack::stack_entry;
    friend class gc_new_stack::activation_entry;
private:
    void push_offset(size_t offset)
    {
        if (m_top_offsets_cnt == m_top_offsets.size()) {
            utils::dynarray<size_t> new_offsets(2 * m_top_offsets_cnt);
            std::copy(m_top_offsets.begin(), m_top_offsets.end(), new_offsets.begin());
            m_top_offsets = new_offsets;
        }
        m_top_offsets[m_top_offsets_cnt++] = offset;
    }

    void* m_top_ptr;
    size_t m_top_size;
    size_t m_depth;
    bool m_is_meta_requested;
    size_t m_top_offsets_cnt;
    gc_new_stack::offsets_storage_t m_top_offsets;
};

static thread_local stack_top top{};

gc_new_stack::stack_entry::stack_entry(void* new_ptr, size_t new_size)
    : m_old_offsets(START_OFFSETS_STORAGE_SIZE)
{
    assert(top.is_active());

    m_old_ptr = top.m_top_ptr;
    top.m_top_ptr = new_ptr;

    m_old_size = top.m_top_size;
    top.m_top_size = new_size;

    m_old_is_meta_requested = top.m_is_meta_requested;
    top.m_is_meta_requested = true;

    m_old_offsets_cnt = top.m_top_offsets_cnt;
    top.m_top_offsets_cnt = 0;

    m_old_offsets.swap(top.m_top_offsets);
}

gc_new_stack::stack_entry::~stack_entry()
{
    top.m_top_ptr = m_old_ptr;
    top.m_top_size = m_old_size;
    top.m_is_meta_requested = m_old_is_meta_requested;
    top.m_top_offsets_cnt = m_old_offsets_cnt;
    top.m_top_offsets.swap(m_old_offsets);
}

gc_new_stack::activation_entry::activation_entry()
{
    top.m_depth++;
}

gc_new_stack::activation_entry::~activation_entry()
{
    top.m_depth--;
}

void gc_new_stack::register_child(gc_untyped_ptr* child)
{
    top.register_child(child);
}

gc_new_stack::offsets_range gc_new_stack::offsets()
{
    return top.offsets();
}

size_t gc_new_stack::depth() noexcept
{
    return top.depth();
}

bool gc_new_stack::is_active() noexcept
{
    return top.is_active();
}

bool gc_new_stack::is_meta_requsted() noexcept
{
    return top.is_meta_requsted();
}

}}}