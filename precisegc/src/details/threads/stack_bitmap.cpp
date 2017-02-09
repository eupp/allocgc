#include <libprecisegc/details/collectors/stack_bitmap.hpp>

#include <cassert>

#include <libprecisegc/details/threads/return_address.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace collectors {

stack_bitmap::stack_bitmap(byte* stack_start_addr)
    : m_stack_start(stack_start_addr)
    , m_stack_end(stack_start_addr + STACK_DIRECTION * threads::stack_maxsize())
{
    logging::info() << "stack start addr=" << (void*) m_stack_start
                    << "; stack end addr=" << (void*) m_stack_end;
}

void stack_bitmap::register_root(gc_handle* root)
{
    auto idxs = root_idxs(root);
    if (idxs.first >= m_bitmap.size()) {
        m_bitmap.resize(idxs.first + 1);
    }
    m_bitmap[idxs.first].set(idxs.second);
}

void stack_bitmap::deregister_root(gc_handle* root)
{
    assert(contains(root));

    auto idxs = root_idxs(root);
    m_bitmap[idxs.first].reset(idxs.second);
}

void stack_bitmap::trace(const gc_trace_callback& cb) const
{
    gc_handle* it = reinterpret_cast<gc_handle*>(m_stack_start);
    for (auto& bitmap_frame: m_bitmap) {
        for (size_t i = 0; i < bitmap_frame.size(); ++i) {
            if (bitmap_frame.test(i)) {
                cb(it);
            }
            STACK_DIRECTION == stack_growth_direction::UP ? ++it : --it;
        }
    }
}

size_t stack_bitmap::size() const
{
    size_t cnt = 0;
    for (auto& bitmap_frame: m_bitmap) {
        cnt += bitmap_frame.count();
    }
    return cnt;
}

bool stack_bitmap::contains(const gc_handle* ptr) const
{
    auto idxs = root_idxs(ptr);
    if (idxs.first >= m_bitmap.size()) {
        return false;
    }
    return m_bitmap[idxs.first].test(idxs.second);
}

std::pair<size_t, size_t> stack_bitmap::root_idxs(const gc_handle* ptr) const
{
    ptrdiff_t diff = STACK_DIRECTION * (reinterpret_cast<const byte*>(ptr) - m_stack_start);
    assert(diff >= 0);
    size_t idx = static_cast<size_t>(diff) >> GC_HANDLE_SIZE_LOG2;
    assert((idx & STACK_FRAME_MASK) < STACK_FRAME_SIZE);
    return std::make_pair(idx >> STACK_FRAME_SIZE_LOG2, idx & STACK_FRAME_MASK);
}




}}}