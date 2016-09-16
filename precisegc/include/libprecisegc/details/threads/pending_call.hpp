#ifndef DIPLOMA_PENDING_CALL_HPP
#define DIPLOMA_PENDING_CALL_HPP

#include <csignal>
#include <atomic>
#include <array>

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class pending_call : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef void (*callable_type)(void);

    constexpr pending_call(callable_type callable)
        : m_callable(callable)
        , m_depth(0)
        , m_pending_flag(NOT_PENDING)
        , m_depth_stack_top(0)
        , m_depth_stack()
    {}

    void operator()();

    void enter_pending_scope();
    void leave_pending_scope();
    
    void enter_safe_scope();
    void leave_safe_scope();

    bool is_in_pending_scope() const;
private:
    static const size_t DEPTH_STACK_SIZE = 8;

    void call_if_pended();

    static const size_t PENDING = 1;
    static const size_t NOT_PENDING = 0;

    callable_type m_callable;
    std::atomic<size_t> m_depth;
    std::atomic<size_t> m_pending_flag;
    std::atomic<size_t> m_depth_stack_top;
    std::array<size_t, DEPTH_STACK_SIZE> m_depth_stack;
};

}}}

#endif //DIPLOMA_PENDING_CALL_HPP
