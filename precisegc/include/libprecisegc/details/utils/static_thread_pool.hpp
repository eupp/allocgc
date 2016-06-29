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
        , m_done(false)
    {
        auto guard = make_scope_guard([this] { stop_workers(); });
        for (auto& thread: m_threads) {
            thread = std::thread(&precisegc::details::utils::static_thread_pool::thread_routine, this);
        }
        guard.commit();
    }

    ~static_thread_pool()
    {
        stop_workers();
    }

    template <typename Functor>
    void submit(Functor&& f)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        unsafe_submit(std::forward<Functor>(f));
        m_not_empty_cond.notify_one();
    };

    template <typename Iterator>
    void submit(Iterator first, Iterator last)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (; first != last; ++first) {
            unsafe_submit(*first);
        }
        m_not_empty_cond.notify_all();
    }

    void wait_for_complete()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_empty_cond.wait(lock, [this] { return m_tasks.empty(); });
    }
private:
    template <typename Functor>
    void unsafe_submit(Functor&& f)
    {
        static_assert(std::is_same<void, typename std::result_of<Functor()>::type>::value,
                      "Only void() tasks are supported");
        m_tasks.emplace(std::forward<Functor>(f));
    }

    void stop_workers()
    {
        m_done = true;
        m_not_empty_cond.notify_all();
    }

    void thread_routine()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_not_empty_cond.wait(lock, [this] { return !m_tasks.empty() || m_done; });
            if (m_done) {
                return;
            }
            std::function<void()> task = m_tasks.front();
            m_tasks.pop();
            m_empty_cond.notify_all();
            lock.unlock();
            task();
        }
    }

    dynarray<scoped_thread> m_threads;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_not_empty_cond;
    std::condition_variable m_empty_cond;
    std::atomic<bool> m_done;
};

}}}

#endif //DIPLOMA_THREAD_POOL_HPP
