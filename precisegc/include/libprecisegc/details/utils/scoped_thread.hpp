#ifndef DIPLOMA_SCOPED_THREAD_HPP
#define DIPLOMA_SCOPED_THREAD_HPP

#include <thread>
#include <stdexcept>

#include <libprecisegc/details/util.h>

namespace precisegc { namespace details { namespace utils {

class scoped_thread : private noncopyable
{
public:
    scoped_thread() = default;
    scoped_thread(scoped_thread&&) = default;

    scoped_thread& operator=(scoped_thread&&) = default;

    scoped_thread(std::thread&& thread)
        : m_thread(std::move(thread))
    {}

    scoped_thread& operator=(std::thread&& thread)
    {
        m_thread = std::move(thread);
        return *this;
    }

    ~scoped_thread()
    {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    bool joinable() const
    {
        return m_thread.joinable();
    }

    std::thread::id get_id() const
    {
        return m_thread.get_id();
    }

    std::thread::native_handle_type native_handle()
    {
        return m_thread.native_handle();
    }

    void join()
    {
        m_thread.join();
    }

    void detach()
    {
        m_thread.detach();
    }

    void swap(scoped_thread& other)
    {
        m_thread.swap(other.m_thread);
    }
private:
    std::thread m_thread;
};

}}}

#endif //DIPLOMA_SCOPED_THREAD_HPP
