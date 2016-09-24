#include <libprecisegc/details/threads/stack_bitmap.hpp>

#include <cassert>

#include <libprecisegc/details/threads/return_address.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace threads {

stack_bitmap::stack_bitmap(byte* stack_start_addr)
    : m_stack_start(stack_start_addr)
    , m_stack_end(stack_start_addr + stack_maxsize())
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

bool stack_bitmap::contains(const gc_handle* ptr) const
{
    auto idxs = root_idxs(ptr);
    if (idxs.first >= m_bitmap.size()) {
        return false;
    }
    return m_bitmap[idxs.first].test(idxs.second);
}

size_t stack_bitmap::count() const
{
    size_t cnt = 0;
    for (auto& bitmap_frame: m_bitmap) {
        cnt += bitmap_frame.count();
    }
    return cnt;
}

std::pair<size_t, size_t> stack_bitmap::root_idxs(const gc_handle* ptr) const
{
    ptrdiff_t diff = STACK_DIRECTION * (reinterpret_cast<const byte*>(ptr) - m_stack_start);
    assert(diff >= 0);
    size_t idx = static_cast<size_t>(diff) >> GC_HANDLE_SIZE_LOG2;
    assert(idx & STACK_FRAME_MASK < STACK_FRAME_SIZE);
    return std::make_pair(idx >> STACK_FRAME_SIZE_LOG2, idx & STACK_FRAME_MASK);
}




}}}