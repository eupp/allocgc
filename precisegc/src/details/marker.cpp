#include <libprecisegc/details/marker.hpp>

#include <cassert>

#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/root_set.hpp>

namespace precisegc { namespace details {

marker::queue_chunk::queue_chunk()
    : m_size(0)
{}

void marker::queue_chunk::push(ptrs::gc_untyped_ptr* p)
{
    assert(!is_full());
    m_data[m_size++] = p;
}

ptrs::gc_untyped_ptr* marker::queue_chunk::pop()
{
    assert(m_size > 0);
    return m_data[--m_size];
}

bool marker::queue_chunk::is_full() const
{
    return m_size == SIZE;
}

bool marker::queue_chunk::empty() const
{
    return m_size == 0;
}

void marker::worker::routine(marker* m)
{
    worker wrk(m);
    wrk.mark();
    std::lock_guard<std::mutex> lock(m->m_markers_mutex);
    --m->m_markers_cnt;
    m->m_markers_cond.notify_all();
}

marker::worker::worker(marker* m)
    : m_marker(m)
{}

void marker::worker::mark()
{
    void* p = nullptr;
    while (m_marker->m_mark_flag && pop(p)) {
        trace_ptr(p, *this);
    }
}

void marker::worker::push(void* p)
{
    m_local_stack.push_back(p);
}

bool marker::worker::pop(void*& p)
{
    if (m_local_stack.empty()) {
        std::lock_guard<std::mutex> lock(m_marker->m_stack_mutex);
        if (m_marker->m_stack.empty()) {
            m_marker->non_blocking_trace_barrier_buffers();
        }
        if (!m_marker->m_stack.empty()) {
            static const size_t CHUNK_MAXCOUNT = 2048;
            size_t chunk_count = std::min(m_marker->m_stack.size(), CHUNK_MAXCOUNT);
            auto first = m_marker->m_stack.rbegin();
            auto last = std::next(first, chunk_count);
            m_local_stack.insert(m_local_stack.begin(), first, last);
            m_marker->m_stack.resize(m_marker->m_stack.size() - chunk_count);
        } else {
            return false;
        }
    }
    p = m_local_stack.back();
    m_local_stack.pop_back();
    return true;
}

marker::marker()
    : m_markers_cnt(0)
    , m_mark_flag(false)
{}

void marker::trace_roots(const threads::world_snapshot& snapshot)
{
    std::lock_guard<std::mutex> lock(m_stack_mutex);
    snapshot.trace_roots([this] (void* p) {
        non_blocking_push(p);
    });
}

void marker::trace_pins(const threads::world_snapshot& snapshot)
{
    std::lock_guard<std::mutex> lock(m_stack_mutex);
    snapshot.trace_pins([this] (void* p) {
        set_object_pin(p, true);
        non_blocking_push(p);
    });
}

void marker::trace_barrier_buffers()
{
    std::lock_guard<std::mutex> lock(m_stack_mutex);
    non_blocking_trace_barrier_buffers();
}

void marker::non_blocking_trace_barrier_buffers()
{
    auto threads_rng = threads::internals::thread_manager_access::get_managed_threads(threads::thread_manager::instance());
    for (auto thread: threads_rng) {
        auto& buf = thread->get_barrier_buffer();
        void* p = nullptr;
        while (buf.pop(p)) {
            non_blocking_push(p);
        }
    }
}

void marker::start_marking()
{
    static const size_t WORKERS_CNT = 1;

    std::lock_guard<std::mutex> lock(m_markers_mutex);
    assert(m_workers.empty());
    assert(m_markers_cnt == 0);
    m_mark_flag.store(true);
    m_workers.resize(WORKERS_CNT);
    for (size_t i = 0; i < WORKERS_CNT; ++i) {
        m_workers[i] = std::thread(worker::routine, this);
    }
    m_markers_cnt = WORKERS_CNT;
    m_markers_cond.notify_all();
}

void marker::pause_marking()
{
    m_mark_flag.store(false);
    wait_for_marking();
}

void marker::join_markers()
{
    std::unique_lock<std::mutex> lock(m_markers_mutex);
    m_markers_cnt++;
    m_markers_cond.notify_all();
    lock.unlock();
    worker::routine(this);
}

void marker::mark()
{
    std::unique_lock<std::mutex> lock(m_markers_mutex);
    m_markers_cnt++;
    m_markers_cond.notify_all();
    lock.unlock();

    bool flag = m_mark_flag.load();
    m_mark_flag.store(true);
    worker::routine(this);
    m_mark_flag.store(flag);
}

void marker::wait_for_marking()
{
    std::unique_lock<std::mutex> lock(m_markers_mutex);
    m_markers_cond.wait(lock, [this] { return m_markers_cnt == 0; });
    for (auto& thread: m_workers) {
        thread.join();
    }
    m_workers.resize(0);
}

//void marker::push_queue_chunk(std::unique_ptr<marker::queue_chunk>&& chunk)
//{
//    std::lock_guard<std::mutex> lock(m_stack_mutex);
//    m_stack.emplace_back(std::move(chunk));
//}
//
//std::unique_ptr<marker::queue_chunk> marker::pop_queue_chunk()
//{
//    std::lock_guard<std::mutex> lock(m_stack_mutex);
//    if (m_stack.empty()) {
//        non_blocking_trace_barrier_buffers();
//        if (m_stack.empty()) {
//            return std::unique_ptr<marker::queue_chunk>();
//        }
//    }
//    auto chunk = std::move(m_stack.back());
//    m_stack.pop_back();
//    return chunk;
//}

void marker::non_blocking_push(void* p)
{
    m_stack.push_back(p);
}

}}
