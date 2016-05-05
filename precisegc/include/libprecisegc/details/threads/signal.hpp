#ifndef DIPLOMA_SIGNAL_HPP
#define DIPLOMA_SIGNAL_HPP

#include <array>
#include <bitset>
#include <boost/optional.hpp>

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/threads/pending_call.hpp>

namespace precisegc { namespace details { namespace threads {

class signal : private noncopyable, private nonmovable
{
public:
    typedef pending_call::callable_type handler_type;

    signal(int signum);

    handler_type get_handler() const;
    void set_handler(handler_type handler);

    void send(pthread_t thread);
private:
    int m_signum;
    handler_type m_handler;
};

class signal_holder
{
public:
    static signal& get(int signum);


private:
    static const size_t SIGNAL_COUNT = 32;
    static std::array<boost::optional<signal>, SIGNAL_COUNT> signals;
//    static thread_local std::array<boost::optional<pending_call
};

}}}

#endif //DIPLOMA_SIGNAL_HPP
