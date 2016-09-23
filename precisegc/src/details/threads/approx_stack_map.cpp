#include <libprecisegc/details/threads/approx_stack_map.hpp>

#include <cassert>

#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace threads {

approx_stack_map::approx_stack_map(byte* stack_start_addr)
    : m_stack_start(stack_start_addr)
    , m_stack_bottom(stack_start_addr)
    , m_top_frame(&m_stack_bottom)
    , m_pool(sizeof(stack_frame))
{
    logging::info() << "Stack start addr=" << (void*) stack_start_addr;
}

void approx_stack_map::register_root(gc_handle* root)
{
    stack_frame* frame = m_top_frame.load(std::memory_order_relaxed);
    if (frame->is_upward_than_frame(root)) {
        stack_frame* new_frame = create_frame(root);
        new_frame->set_next(frame);
        new_frame->register_root(root);
        m_top_frame.store(new_frame, std::memory_order_relaxed);
        return;
    } else if (frame->is_upward_than_frame_start(root)) {
        frame->register_root(root);
        return;
    }

    stack_frame* prev = frame;
    frame = frame->next();
    while (frame) {
        if (frame->is_upward_than_frame(root)) {
            stack_frame* new_frame = create_frame(root);
            new_frame->set_next(frame);
            new_frame->register_root(root);
            prev->set_next(new_frame);
            return;
        } else if (frame->is_upward_than_frame_start(root)) {
            frame->register_root(root);
            return;
        }

        prev = frame;
        frame = frame->next();
    }

    assert(false);
}

void approx_stack_map::deregister_root(gc_handle* root)
{
    assert(contains(root));

    stack_frame* frame = m_top_frame.load(std::memory_order_relaxed);
    if (frame->contains(root)) {
        frame->deregister_root(root);
        if (frame->empty() && frame != &m_stack_bottom) {
            m_top_frame.store(frame->next(), std::memory_order_relaxed);
            m_pool.deallocate(reinterpret_cast<byte*>(frame));
        }
        return;
    }

    stack_frame* prev = frame;
    frame = frame->next();
    while (!frame->contains(root)) {
        prev = frame;
        frame = frame->next();
    }

    frame->deregister_root(root);
    if (frame->empty() && frame != &m_stack_bottom) {
        prev->set_next(frame->next());
        m_pool.deallocate(reinterpret_cast<byte*>(frame));
    }
}

bool approx_stack_map::contains(const gc_handle* ptr) const
{
    stack_frame* frame = m_top_frame.load(std::memory_order_relaxed);
    while (frame) {
        if (frame->contains(ptr) && frame->is_registered_root(ptr)) {
            return true;
        }
        frame = frame->next();
    }
    return false;
}

size_t approx_stack_map::count() const
{
    size_t cnt = 0;
    stack_frame* frame = m_top_frame.load(std::memory_order_relaxed);
    while (frame) {
        cnt += frame->count();
        frame = frame->next();
    }
    return cnt;
}

approx_stack_map::stack_frame* approx_stack_map::create_frame(const gc_handle* root)
{
    size_t frame_offset = (stack_diff_in_words(root, m_stack_start) >> STACK_FRAME_SIZE_LOG2) * STACK_FRAME_SIZE * GC_HANDLE_SIZE;
    byte* frame_addr = m_stack_start + STACK_DIRECTION * frame_offset;
    stack_frame* new_frame = reinterpret_cast<stack_frame*>(m_pool.allocate());
    new (new_frame) stack_frame(frame_addr);
    return new_frame;
}

size_t approx_stack_map::stack_diff_in_words(const gc_handle* ptr, byte* stack_addr)
{
    return static_cast<size_t>(
                   STACK_DIRECTION * (reinterpret_cast<const byte*>(ptr) - stack_addr)
           ) >> GC_HANDLE_SIZE_LOG2;
}

approx_stack_map::stack_frame::stack_frame(byte* stack_addr)
    : m_stack_begin(stack_addr)
    , m_stack_end(m_stack_begin + STACK_DIRECTION * GC_HANDLE_SIZE * STACK_FRAME_SIZE)
    , m_next(nullptr)
{}

void approx_stack_map::stack_frame::register_root(gc_handle* root)
{
    assert(contains(root));
    size_t idx = get_root_idx(root);
    m_bitmap.set(idx, true);
}

void approx_stack_map::stack_frame::deregister_root(gc_handle* root)
{
    assert(contains(root));
    size_t idx = get_root_idx(root);
    m_bitmap.set(idx, false);
}

bool approx_stack_map::stack_frame::is_upward_than_frame(const gc_handle* ptr) const
{
    const byte* p = reinterpret_cast<const byte*>(ptr);
    return STACK_DIRECTION == stack_growth_direction::UP
        ? p >= m_stack_end
        : p <= m_stack_end;
}

bool approx_stack_map::stack_frame::is_upward_than_frame_start(const gc_handle* ptr) const
{
    const byte* p = reinterpret_cast<const byte*>(ptr);
    return STACK_DIRECTION == stack_growth_direction::UP
           ? p >= m_stack_begin
           : p <= m_stack_begin;
}

bool approx_stack_map::stack_frame::is_registered_root(const gc_handle* ptr) const
{
    assert(contains(ptr));
//    assert(reinterpret_cast<std::uintptr_t>(ptr) % GC_HANDLE_SIZE == 0);
    size_t idx = get_root_idx(ptr);
    return m_bitmap.test(idx);
}

bool approx_stack_map::stack_frame::contains(const gc_handle* ptr) const
{
    const byte* p = reinterpret_cast<const byte*>(ptr);
    if (STACK_DIRECTION == stack_growth_direction::UP) {
        return (m_stack_begin <= p) && (p < m_stack_end);
    } else {
        return (m_stack_end < p) && (p <= m_stack_begin);
    }
}

bool approx_stack_map::stack_frame::empty() const
{
    return m_bitmap.none();
}

size_t approx_stack_map::stack_frame::count() const
{
    return m_bitmap.count();
}

size_t approx_stack_map::stack_frame::get_root_idx(const gc_handle* root) const
{

    return stack_diff_in_words(root, m_stack_begin);
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