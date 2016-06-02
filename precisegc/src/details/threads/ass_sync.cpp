#include <libprecisegc/details/threads/ass_sync.hpp>

#include <cerrno>

#include <libprecisegc/details/utils/system_error.hpp>

namespace precisegc { namespace details { namespace threads {

namespace internals {

unnamed_pipe::unnamed_pipe()
    : m_pipefd({-1, -1})
{
    int rc = pipe(m_pipefd);
    utils::throw_system_error(rc, "Failed to initialize unnamed_pipe");
}

unnamed_pipe::~unnamed_pipe()
{
    for (auto& pipefd: m_pipefd) {
        int rc = close(pipefd);
        utils::log_system_error(rc, "Failed to destroy unnamed_pipe");
    }
}

int unnamed_pipe::read_fd() const
{
    return m_pipefd[0];
}

int unnamed_pipe::write_fd() const
{
    return m_pipefd[1];
}

}

void ass_event::wait()
{
    ssize_t cnt = 0;
    char byte = 0;
    while (cnt != 1) {
        cnt = read(m_pipe.read_fd(), &byte, 1);
        utils::log_system_error(cnt, "ass_event::wait() error");
    }
}

void ass_event::notify(size_t cnt)
{
    static const size_t BUF_SIZE = 64;
    static const char buf[BUF_SIZE] = {0};

    size_t bytes_written = 0;
    size_t curr_cnt = cnt;
    while  (bytes_written < cnt) {
        size_t bytes_to_write = std::min(curr_cnt, BUF_SIZE);
        ssize_t res = write(m_pipe.write_fd(), buf, bytes_to_write);
        if (res != -1) {
            bytes_written += res;
            curr_cnt -= res;
        }
        utils::log_system_error(res, "ass_event::notify() error");
    }
}

void ass_barrier::wait(size_t cnt)
{
    static const size_t BUF_SIZE = 64;
    static char buf[BUF_SIZE] = {0};

    size_t bytes_read = 0;
    size_t curr_cnt = cnt;
    while  (bytes_read < cnt) {
        size_t bytes_to_read = std::min(curr_cnt, BUF_SIZE);
        ssize_t res = read(m_pipe.read_fd(), buf, bytes_to_read);
        if (res != -1) {
            bytes_read += res;
            curr_cnt -= res;
        }
        utils::log_system_error(res, "ass_barrier::wait() error");
    }
}

void ass_barrier::notify()
{
    ssize_t cnt = 0;
    char byte = 0;
    while (cnt != 1) {
        cnt = write(m_pipe.write_fd(), &byte, 1);
        utils::log_system_error(cnt, "ass_barrier::notify() error");
    }
}

posix_signal& ass_mutex::sig = posix_signal::instance();

void ass_mutex::lock()
{
    sig.lock();
    m_mutex.lock();
}

void ass_mutex::unlock()
{
    sig.unlock();
    m_mutex.unlock();
}

bool ass_mutex::try_lock()
{
    sig.lock();
    if (m_mutex.try_lock()) {
        return true;
    } else {
        sig.unlock();
        return false;
    }
}

}}}

