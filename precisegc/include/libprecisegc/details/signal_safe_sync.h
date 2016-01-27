#ifndef DIPLOMA_SIGNAL_SAFE_SYNC_H
#define DIPLOMA_SIGNAL_SAFE_SYNC_H

#include <cassert>
#include <algorithm>
#include <iostream>
#include <unistd.h>

#include "noncopyable.h"

namespace precisegc { namespace details {

class signal_safe_event: public noncopyable
{
public:
    signal_safe_event()
            : m_pipefd({-1, -1})
    {
        int res = pipe(m_pipefd);
        assert(res == 0);
    }


    void wait()
    {
        ssize_t cnt = 0;
        char byte = 0;
        while (cnt != 1) {
            cnt = read(m_pipefd[0], &byte, 1);
        }
    }

    void notify(size_t cnt)
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

private:
    int m_pipefd[2];
};

class signal_safe_barrier: public noncopyable
{
public:
    signal_safe_barrier()
            : m_pipefd({-1, -1})
    {
        int res = pipe(m_pipefd);
        assert(res == 0);
    }

    void wait(size_t cnt)
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

    void notify()
    {
        ssize_t cnt = 0;
        char byte = 0;
        while (cnt != 1) {
            cnt = write(m_pipefd[1], &byte, 1);
        }
    }

private:
    int m_pipefd[2];
};

}}

#endif //DIPLOMA_SIGNAL_SAFE_SYNC_H
