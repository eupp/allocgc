#include "signal_safe_sync.h"

#include "gc_pause.h"

namespace precisegc { namespace details {

signal_safe_event::signal_safe_event()
    : m_pipefd({-1, -1})
{
    int res = pipe(m_pipefd);
    assert(res == 0);
}

signal_safe_event::~signal_safe_event()
{
    close(m_pipefd[0]);
    close(m_pipefd[1]);
}

void signal_safe_event::wait()
{
    ssize_t cnt = 0;
    char byte = 0;
    while (cnt != 1) {
        cnt = read(m_pipefd[0], &byte, 1);
    }
}

void signal_safe_event::notify(size_t cnt)
{
    static const size_t BUF_SIZE = 64;
    static const char buf[BUF_SIZE] = {0};

    size_t bytes_written = 0;
    size_t bytes_to_write = std::min(cnt, BUF_SIZE);
    while  (bytes_written < cnt) {
        ssize_t res = write(m_pipefd[1], buf, bytes_to_write);
        if (res != -1) {
            bytes_written += res;
        }
    }
}

signal_safe_barrier::signal_safe_barrier()
    : m_pipefd({-1, -1})
{
    int res = pipe(m_pipefd);
    assert(res == 0);
}

signal_safe_barrier::~signal_safe_barrier()
{
    close(m_pipefd[0]);
    close(m_pipefd[1]);
}

void signal_safe_barrier::wait(size_t cnt)
{
    static const size_t BUF_SIZE = 64;
    static char buf[BUF_SIZE] = {0};

    size_t bytes_read = 0;
    size_t bytes_to_read = std::min(cnt, BUF_SIZE);
    while  (bytes_read < cnt) {
        ssize_t res = read(m_pipefd[0], buf, bytes_to_read);
        if (res != -1) {
            bytes_read += res;
        }
    }
}

size_t signal_safe_barrier::wait_for(size_t cnt, timeval* tv)
{
    static const size_t BUF_SIZE = 64;
    static char buf[BUF_SIZE] = {0};

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(m_pipefd[0], &rfds);

    size_t bytes_read = 0;
    size_t bytes_to_read = std::min(cnt, BUF_SIZE);
    int res = select(m_pipefd[0] + 1, &rfds, nullptr, nullptr, tv);
    while (res == 1 && bytes_read != cnt) {
        ssize_t read_res = read(m_pipefd[0], buf, bytes_to_read);
        if (read_res != -1) {
            bytes_read += read_res;

            timeval null_tv;
            null_tv.tv_sec = 0;
            null_tv.tv_usec = 0;
            res = select(m_pipefd[0] + 1, &rfds, nullptr, nullptr, &null_tv);
        } else {
            break;
        }
    }
    return bytes_read;
}

void signal_safe_barrier::notify()
{
    ssize_t cnt = 0;
    char byte = 0;
    while (cnt != 1) {
        cnt = write(m_pipefd[1], &byte, 1);
    }
}

thread_local std::atomic<size_t> signal_safe_mutex::depth(0);

void signal_safe_mutex::lock() noexcept
{
    if (depth == 0) {
        disable_gc_pause();
    }
    ++depth;
    m_mutex.lock();
}

void signal_safe_mutex::unlock() noexcept
{
    m_mutex.unlock();
    --depth;
    if (depth == 0) {
        enable_gc_pause();
    }
}

}}
