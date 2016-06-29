#ifndef DIPLOMA_THREAD_POOL_HPP
#define DIPLOMA_THREAD_POOL_HPP

#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <utility>
#include <type_traits>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/utils/dynarray.hpp>

namespace precisegc { namespace details { namespace utils {

class static_thread_pool : private utils::noncopyable
{
public:
    explicit static_thread_pool(size_t n)
        : m_threads(n)
    {
        auto guard = make_scope_guard([this] { m_done = true; });
        for (auto& thread: m_threads) {
            thread = std::thread(thread_routine, this);
        }
        guard.commit();
    }

    ~static_thread_pool()
    {
        m_done = false;
    }

    template <typename Functor>
    void submit(Functor&& f)
    {
        static_assert(std::is_same<void, typename std::result_of<Functor()>::type>::value,
                      "Only void() tasks are supported");
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.emplace(std::forward<Functor>(f));
        m_condvar.notify_one();
    };

    void wait_for_all_tasks()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condvar.wait(lock, [this] { return m_tasks.empty(); });
    }
private:
    void thread_routine()
    {
        while (!m_done) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condvar.wait(lock, [this] { return !m_tasks.empty(); });
            std::function<void()> task = m_tasks.front();
            m_tasks.pop();
            lock.unlock();
            task();
        }
    }

    dynarray<scoped_thread> m_threads;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condvar;
    std::atomic<bool> m_done;
};

}}}

#endif //DIPLOMA_THREAD_POOL_HPP
