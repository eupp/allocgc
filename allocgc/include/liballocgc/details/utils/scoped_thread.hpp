#ifndef ALLOCGC_SCOPED_THREAD_HPP
#define ALLOCGC_SCOPED_THREAD_HPP

#include <thread>
#include <utility>

#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace utils {

class scoped_thread : private utils::noncopyable
{
public:
    scoped_thread() = default;
    scoped_thread(scoped_thread&&) = default;

    template <typename Functor, typename... Args>
    explicit scoped_thread(Functor&& f, Args&&... args)
        : m_thread(std::forward<Functor>(f), std::forward<Args>(args)...)
    {};

    scoped_thread(std::thread&& thread)
        : m_thread(std::move(thread))
    {}

    scoped_thread& operator=(scoped_thread thread)
    {
        thread.swap(*this);
        return *this;
    }

    scoped_thread& operator=(std::thread&& thread)
    {
        scoped_thread(std::move(thread)).swap(*this);
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

#endif //ALLOCGC_SCOPED_THREAD_HPP
