#include <liballocgc/details/threads/posix_signal.hpp>

#include <cassert>
#include <cstring>

namespace allocgc { namespace details { namespace threads {

void call_signal_handler()
{
    posix_signal& sig = posix_signal::instance();
    if (sig.m_handler) {
        sig.m_handler();
    }
}

static thread_local pending_call pcall{call_signal_handler};

extern "C" {
void sighandler(int signum)
{
    assert(signum == posix_signal::SIGNUM);
    pcall();
}
}

posix_signal::posix_signal()
    : m_handler(nullptr)
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, posix_signal::SIGNUM);

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = ::allocgc::details::threads::sighandler;
    sa.sa_mask = sigset;
    sa.sa_flags |= SA_RESTART;

    int sa_ret = sigaction(posix_signal::SIGNUM, &sa, nullptr);
    assert(sa_ret == 0);
}

posix_signal& posix_signal::instance()
{
    static posix_signal ps;
    return ps;
}

posix_signal::handler_type posix_signal::get_handler() const
{
    return m_handler;
}

void posix_signal::set_handler(handler_type handler)
{
    m_handler = handler;
}

void posix_signal::send(pthread_t thread)
{
    pthread_kill(thread, SIGNUM);
}

void posix_signal::lock()
{
    pcall.enter_pending_scope();
}

void posix_signal::unlock()
{
    pcall.leave_pending_scope();
}

void posix_signal::enter_safe_scope()
{
    pcall.enter_safe_scope();
}

void posix_signal::leave_safe_scope()
{
    pcall.leave_safe_scope();
}

bool posix_signal::is_locked()
{
    return pcall.is_in_pending_scope();
}

}}}

