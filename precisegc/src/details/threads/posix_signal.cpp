#include <libprecisegc/details/threads/posix_signal.hpp>

#include <cassert>
#include <cstring>

namespace precisegc { namespace details { namespace threads {

void posix_signal::call_signal_handler()
{
    posix_signal& sig = posix_signal::instance();
    if (sig.m_handler) {
        sig.m_handler();
    }
}

thread_local pending_call posix_signal::pcall(posix_signal::call_signal_handler);

extern "C" {
void sighandler(int signum)
{
    assert(signum == posix_signal::SIGNUM);
    posix_signal::pcall();
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
    sa.sa_handler = ::precisegc::details::threads::sighandler;
    sa.sa_mask = sigset;

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
    posix_signal::pcall.lock();
}

void posix_signal::unlock()
{
    posix_signal::pcall.unlock();
}

}}}

