#ifndef DIPLOMA_OS_THREAD_HPP
#define DIPLOMA_OS_THREAD_HPP

#include <pthread.h>

#include <libprecisegc/details/threads/pending_call.hpp>

namespace precisegc { namespace details { namespace threads {

class posix_thread
{
public:


private:
    pthread_t m_pthread;
    pending_call m_stw_sighandler;
};

}}}

#endif //DIPLOMA_OS_THREAD_HPP
