#include <liballocgc/details/threads/pending_call.hpp>

#include <cassert>

namespace allocgc { namespace details { namespace threads {

void pending_call::operator()()
{
    assert(m_callable);
    if (m_depth.load(std::memory_order_relaxed) > 0) {
        m_pending_flag.store(PENDING, std::memory_order_relaxed);
        return;
    }
    m_callable();
}

void pending_call::enter_pending_scope()
{
    m_depth.store(m_depth.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
}

void pending_call::leave_pending_scope()
{
    assert(m_callable);
    assert(m_depth > 0);
    size_t depth = m_depth.load(std::memory_order_relaxed);
    if (depth == 1) {
        m_depth.store(0, std::memory_order_relaxed);
        call_if_pended();
    } else {
        m_depth.store(depth - 1, std::memory_order_relaxed);
    }
}

void pending_call::enter_safe_scope()
{
    assert(m_depth_stack_top < DEPTH_STACK_SIZE);
    size_t i = m_depth_stack_top.load(std::memory_order_relaxed);
    m_depth_stack[i] = m_depth.load(std::memory_order_relaxed);
    m_depth_stack_top.store(i + 1, std::memory_order_relaxed);
    m_depth.store(0, std::memory_order_relaxed);
    call_if_pended();
}

void pending_call::leave_safe_scope()
{
    assert(m_depth_stack_top > 0);
    size_t i = m_depth_stack_top.load(std::memory_order_relaxed) - 1;
    m_depth.store(m_depth_stack[i], std::memory_order_relaxed);
    m_depth_stack_top.store(i, std::memory_order_relaxed);
}

bool pending_call::is_in_pending_scope() const
{
    return m_depth.load(std::memory_order_relaxed) > 0;
}

void pending_call::call_if_pended()
{
    bool pending = (m_pending_flag.load(std::memory_order_relaxed) == PENDING);
    m_pending_flag.store(NOT_PENDING, std::memory_order_relaxed);
    if (pending) {
        m_callable();
    }
}

}}}

