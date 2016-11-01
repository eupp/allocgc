#ifndef DIPLOMA_THREAD_POOL_HPP
#define DIPLOMA_THREAD_POOL_HPP

#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <utility>
#include <type_traits>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/utils/dynarray.hpp>

namespace precisegc { namespace details { namespace utils {

class static_thread_pool : private utils::noncopyable, private utils::nonmovable
{
public:
    explicit static_thread_pool(size_t n)
        : m_threads(n)
        , m_done(false)
        , m_tasks_cnt(0)
        , m_complete_tasks_cnt(0)
        , m_tasks_ready(false)
    {
        auto guard = make_scope_guard([this] { stop_workers(); });
        for (size_t i = 0; i < m_threads.size(); ++i) {
            m_threads[i] = std::thread(&precisegc::details::utils::static_thread_pool::thread_routine, this, i);
        }
        guard.commit();
    }

    ~static_thread_pool()
    {
        stop_workers();
    }

    template <typename Iterator>
    void run(Iterator first, Iterator last)
    {
        typedef decltype(*std::declval<Iterator>()) functor_t;
        static_assert(std::is_same<void, typename std::result_of<functor_t()>::type>::value,
                      "Only void() tasks are supported");

        if (m_threads.size() == 0) {
            for (; first != last; ++first) {
                (*first)();
            }
            return;
        }

        std::unique_lock<std::mutex> lock(m_mutex);
        for (; first != last; ++first) {
            m_tasks.emplace_back(*first);
        }
        m_tasks_cnt = m_tasks.size();
        m_complete_tasks_cnt = 0;
        m_tasks_ready = true;
        m_tasks_ready_cond.notify_all();

        m_tasks_complete_cond.wait(lock, [this] { return m_complete_tasks_cnt == m_tasks_cnt; });
        m_tasks_ready = false;
    }
private:
    void stop_workers()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_done = true;
        m_tasks_ready_cond.notify_all();
    }

    void thread_routine(size_t thread_num)
    {
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_tasks_ready_cond.wait(lock, [this] { return (m_tasks_ready && !m_tasks.empty()) || m_done; });
            if (m_done) {
                return;
            }

            size_t tasks_per_worker = m_tasks_cnt / m_threads.size();
            tasks_per_worker += thread_num < (m_tasks_cnt % m_threads.size()) ? 1 : 0;
            std::vector<std::function<void()>> worker_tasks(m_tasks.rbegin(), std::next(m_tasks.rbegin(), tasks_per_worker));
            m_tasks.resize(m_tasks.size() - tasks_per_worker);
            lock.unlock();

            for (auto& task: worker_tasks) {
                task();
            }

            lock.lock();
            m_complete_tasks_cnt += tasks_per_worker;
            m_tasks_complete_cond.notify_all();
            m_tasks_complete_cond.wait(lock, [this] { return m_complete_tasks_cnt == m_tasks_cnt; });
        }
    }

    dynarray<scoped_thread> m_threads;
    std::vector<std::function<void()>> m_tasks;
    size_t m_tasks_cnt;
    size_t m_complete_tasks_cnt;
    std::mutex m_mutex;
    std::condition_variable m_tasks_ready_cond;
    std::condition_variable m_tasks_complete_cond;
    bool m_tasks_ready;
    bool m_done;
};

}}}

#endif //DIPLOMA_THREAD_POOL_HPP
