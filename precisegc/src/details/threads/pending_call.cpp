#include <libprecisegc/details/threads/pending_call.hpp>

#include <cassert>
#include <atomic>

namespace precisegc { namespace details { namespace threads {

void pending_call::operator()()
{
    assert(m_callable);
    std::atomic_signal_fence(std::memory_order_seq_cst);
    if (m_depth > 0) {
        m_pending_flag = PENDING;
        std::atomic_signal_fence(std::memory_order_seq_cst);
        return;
    }
    m_callable();
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

void pending_call::enter_pending_scope()
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
    m_depth++;
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

void pending_call::leave_pending_scope()
{
    assert(m_callable);
    std::atomic_signal_fence(std::memory_order_seq_cst);
    if (m_depth == 1) {
        m_depth = 0;
        call_if_pended();
    } else {
        m_depth--;
    }
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

void pending_call::enter_safe_scope()
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
    m_saved_depth = m_depth;
    m_depth = 0;
    call_if_pended();
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

void pending_call::leave_safe_scope()
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
    m_depth = m_saved_depth;
    m_saved_depth = 0;
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

bool pending_call::is_in_pending_scope() const
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
    return m_depth > 0;
}

void pending_call::call_if_pended()
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
    bool pending = (m_pending_flag == PENDING);
    m_pending_flag = NOT_PENDING;
    if (pending) {
        m_callable();
    }
}

}}}

