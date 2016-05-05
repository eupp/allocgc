#include <libprecisegc/details/threads/pending_call.hpp>

#include <cassert>
#include <atomic>

namespace precisegc { namespace details { namespace threads {

pending_call::pending_call(callable_type callable)
    : m_callable(callable)
    , m_depth(0)
    , m_pending_flag(NOT_PENDING)
{}

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
}

void pending_call::lock()
{
    if (m_depth == 0) {
        std::atomic_signal_fence(std::memory_order_seq_cst);
        m_depth = 1;
        std::atomic_signal_fence(std::memory_order_seq_cst);
    } else {
        m_depth++;
    }
}

void pending_call::unlock()
{
    assert(m_callable);
    if (m_depth == 1) {
        std::atomic_signal_fence(std::memory_order_seq_cst);
        m_depth = 0;
        std::atomic_signal_fence(std::memory_order_seq_cst);
        bool pending = (m_pending_flag == PENDING);
        m_pending_flag = NOT_PENDING;
        if (pending) {
            m_callable();
        }
    } else {
        m_depth--;
    }
}

}}}

