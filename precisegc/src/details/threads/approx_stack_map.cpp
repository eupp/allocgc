#include <libprecisegc/details/threads/approx_stack_map.hpp>

#include <cassert>

#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace threads {

approx_stack_map::approx_stack_map(byte* stack_start_addr)
    : m_stack_start(stack_start_addr)
{
    logging::info() << "Stack start addr=" << (void*) stack_start_addr;
}

void approx_stack_map::register_root(gc_handle* root)
{
    auto idxs = root_idxs(root);
    if (idxs.first >= m_bitmap.size()) {
        m_bitmap.resize(idxs.first + 1);
    }
    m_bitmap[idxs.first].set(idxs.second);
}

void approx_stack_map::deregister_root(gc_handle* root)
{
    assert(contains(root));

    auto idxs = root_idxs(root);
    m_bitmap[idxs.first].reset(idxs.second);
}

bool approx_stack_map::contains(const gc_handle* ptr) const
{
    auto idxs = root_idxs(ptr);
    if (idxs.first >= m_bitmap.size()) {
        return false;
    }
    return m_bitmap[idxs.first].test(idxs.second);
}

size_t approx_stack_map::count() const
{
    size_t cnt = 0;
    for (auto& bitmap_frame: m_bitmap) {
        cnt += bitmap_frame.count();
    }
    return cnt;
}

std::pair<size_t, size_t> approx_stack_map::root_idxs(const gc_handle* ptr) const
{
    ptrdiff_t diff = STACK_DIRECTION * (reinterpret_cast<const byte*>(ptr) - m_stack_start);
    assert(diff >= 0);
    size_t idx = static_cast<size_t>(diff) >> GC_HANDLE_SIZE_LOG2;
    assert(idx & STACK_FRAME_MASK < STACK_FRAME_SIZE);
    return std::make_pair(idx >> STACK_FRAME_SIZE_LOG2, idx & STACK_FRAME_MASK);
}




}}}