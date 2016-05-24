#ifndef DIPLOMA_ASS_SYNC_HPP
#define DIPLOMA_ASS_SYNC_HPP

#include <unistd.h>
#include <mutex>

#include <libprecisegc/details/threads/posix_signal.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

namespace internals {
class unnamed_pipe : private utils::noncopyable, private utils::nonmovable
{
public:
    unnamed_pipe();
    ~unnamed_pipe();

    int read_fd() const;
    int write_fd() const;
protected:
    int m_pipefd[2];
};
}

class ass_event : private utils::noncopyable, private utils::nonmovable
{
public:
    ass_event() = default;

    void wait();
    void notify(size_t cnt);
private:
    internals::unnamed_pipe m_pipe;
};

class ass_barrier : public utils::noncopyable, private utils::nonmovable
{
public:
    ass_barrier() = default;

    void wait(size_t cnt);
    void notify();
private:
    internals::unnamed_pipe m_pipe;
};

class ass_mutex : public utils::noncopyable, public utils::nonmovable
{
public:
    ass_mutex() = default;

    void lock();
    bool try_lock();
    void unlock();
private:
    static posix_signal& sig;

    std::mutex m_mutex;
};

}}}

#endif //DIPLOMA_ASS_SYNC_HPP
