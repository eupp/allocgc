#include <libprecisegc/details/threads/approx_stack_map.hpp>

#include <cassert>

namespace precisegc { namespace details { namespace threads {

approx_stack_map::approx_stack_map()
    : m_top(nullptr)
    , m_count(0)
    , m_pool(sizeof(stack_frame))
{}

void approx_stack_map::register_root(gc_handle* root)
{
    stack_frame* top_frame = m_top.load(std::memory_order_relaxed);
    if (!top_frame || root < top_frame->top() || top_frame->is_full()) {
        stack_frame* frame = reinterpret_cast<stack_frame*>(m_pool.allocate());
        new (frame) stack_frame();
        frame->push(root);
        frame->set_next(top_frame);
        m_top.store(frame, std::memory_order_relaxed);
        inc_count();
        return;
    }
    top_frame->push(root);
    inc_count();
}

void approx_stack_map::deregister_root(gc_handle* root)
{
    assert(contains(root));

    dec_count();

    stack_frame* frame = m_top.load(std::memory_order_relaxed);
    if (frame->contains(root)) {
        assert(frame->contains_strict(root));
        if (frame->pop()) {
            m_top.store(frame->next(), std::memory_order_relaxed);
            m_pool.deallocate(reinterpret_cast<byte*>(frame));
        }
        return;
    }

    stack_frame* prev_frame = frame;
    frame = frame->next();
    while (!frame->contains(root)) {
        prev_frame = frame;
        frame = frame->next();
    }
    assert(frame->contains_strict(root));
    if (frame->pop()) {
        prev_frame->set_next(frame->next());
        m_pool.deallocate(reinterpret_cast<byte*>(frame));
    }
}

bool approx_stack_map::contains(const gc_handle* ptr) const
{
    stack_frame* top_frame = m_top.load(std::memory_order_relaxed);
    while (top_frame) {
        if (top_frame->contains_strict(ptr)) {
            return true;
        }
        top_frame = top_frame->next();
    }
    return false;
}

size_t approx_stack_map::count() const
{
    return m_count;
}

void approx_stack_map::inc_count()
{
    m_count.store(m_count.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
}

void approx_stack_map::dec_count()
{
    m_count.store(m_count.load(std::memory_order_relaxed) - 1, std::memory_order_relaxed);
}

approx_stack_map::stack_frame::stack_frame()
    : m_size(0)
    , m_poped_cnt(0)
    , m_next(nullptr)
{}

void approx_stack_map::stack_frame::push(gc_handle* root)
{
    assert(m_size < STACK_FRAME_SIZE);
    size_t size = m_size.load(std::memory_order_relaxed);
    m_data[size] = root;
    m_size.store(size + 1, std::memory_order_relaxed);
}

bool approx_stack_map::stack_frame::pop()
{
    assert(m_size > 0);
    ++m_poped_cnt;
    return m_poped_cnt == m_size.load(std::memory_order_relaxed);
}

gc_handle* approx_stack_map::stack_frame::top() const
{
    assert(m_size > 0);
    return m_data[m_size.load(std::memory_order_relaxed) - 1];
}

bool approx_stack_map::stack_frame::contains(const gc_handle* ptr) const
{
    assert(m_size > 0);
    size_t size = m_size.load(std::memory_order_relaxed) - 1;
    return (m_data[0] <= ptr) && (ptr <= m_data[size]);
}

bool approx_stack_map::stack_frame::contains_strict(const gc_handle* ptr) const
{
    assert(m_size > 0);
    size_t size = m_size.load(std::memory_order_relaxed);
    gc_handle* const* begin = m_data;
    gc_handle* const* end = m_data + size;
    return std::find(begin, end, ptr) != end;
}

bool approx_stack_map::stack_frame::is_full() const
{
    return m_size.load(std::memory_order_relaxed) == STACK_FRAME_SIZE;
}

approx_stack_map::stack_frame* approx_stack_map::stack_frame::next() const
{
    return m_next.load(std::memory_order_relaxed);
}

void approx_stack_map::stack_frame::set_next(stack_frame* next)
{
    m_next.store(next, std::memory_order_relaxed);
}


}}}