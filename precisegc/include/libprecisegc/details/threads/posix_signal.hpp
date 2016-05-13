#ifndef DIPLOMA_SIGNAL_HPP
#define DIPLOMA_SIGNAL_HPP

#include <array>
#include <bitset>
#include <boost/optional.hpp>
#include <signal.h>

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/threads/pending_call.hpp>

namespace precisegc { namespace details { namespace threads {

extern "C" {
void sighandler(int signum);
}

class posix_signal : private noncopyable, private nonmovable
{
public:
    typedef pending_call::callable_type handler_type;

    static posix_signal& instance();

    handler_type get_handler() const;
    void set_handler(handler_type handler);

    void send(pthread_t thread);

    void lock();
    void unlock();

    bool is_locked() const;

    friend void ::precisegc::details::threads::sighandler(int);
private:
    static const int SIGNUM = SIGUSR2;

    static void call_signal_handler();

    static thread_local pending_call pcall;

    posix_signal();

    handler_type m_handler;
};

}}}

#endif //DIPLOMA_SIGNAL_HPP
