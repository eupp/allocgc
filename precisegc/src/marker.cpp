#include <libprecisegc/details/marker.hpp>

#include <cassert>

#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/root_set.hpp>

namespace precisegc { namespace details {

marker::queue_chunk::queue_chunk()
    : m_size(0)
{}

void marker::queue_chunk::push(gc_untyped_ptr* p)
{
    assert(!is_full());
    m_data[m_size++] = p;
}

gc_untyped_ptr* marker::queue_chunk::pop()
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
    : m_curr_queue(0)
    , m_marker(m)
{
    for (auto& chunk: m_local_queue) {
        chunk.reset(new queue_chunk());
    }
}

void marker::worker::mark()
{
    gc_untyped_ptr* p = nullptr;
    while (m_marker->m_mark_flag && pop(p)) {
        trace_ptr(p, *this);
    }
}

void marker::worker::push(gc_untyped_ptr* p)
{
    if (m_local_queue[LOCAL_QUEUE_SIZE - 1]->is_full() && m_local_queue[0]->is_full()) {
        m_marker->push_queue_chunk(std::move(m_local_queue[0]));
        m_local_queue[0].reset(new queue_chunk());
        m_curr_queue = 0;
    } else if (m_local_queue[m_curr_queue]->is_full()) {
        ++m_curr_queue;
    }
    m_local_queue[m_curr_queue]->push(p);
}

bool marker::worker::pop(gc_untyped_ptr*& p)
{
    if (m_local_queue[m_curr_queue]->empty()) {
        if (m_local_queue[--m_curr_queue]->empty()) {
            auto chunk = m_marker->pop_queue_chunk();
            if (!chunk) {
                return false;
            }
            m_local_queue[m_curr_queue] = std::move(chunk);
        }
    }
    p = m_local_queue[m_curr_queue]->pop();
    return true;
}

marker::marker()
    : m_markers_cnt(0)
    , m_mark_flag(false)
{}

void marker::trace_roots()
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    auto threads_rng = threads::thread_manager::instance().get_managed_threads();
    for (auto thread: threads_rng) {
        root_set::element* it = thread->get_root_set().head();
        while (it != nullptr) {
            non_blocking_push(it->root);
            it = it->next;
        }
    }
}

void marker::trace_pins()
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    auto threads_rng = threads::thread_manager::instance().get_managed_threads();
    for (auto thread: threads_rng) {
        root_set::element* it = thread->get_pin_set().head();
        while (it != nullptr) {
            non_blocking_push(it->root);
            set_object_pin(it->root->get(), true);
            it = it->next;
        }
    }
}

void marker::trace_barrier_buffers()
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    non_blocking_trace_barrier_buffers();
}

void marker::non_blocking_trace_barrier_buffers()
{
    auto threads_rng = threads::thread_manager::instance().get_managed_threads();
    for (auto thread: threads_rng) {
        auto& buf = thread->get_barrier_buffer();
        gc_untyped_ptr* p = nullptr;
        while (buf.pop(p)) {
            if (p) {
                non_blocking_push(p);
            }
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
}

void marker::push_queue_chunk(std::unique_ptr<marker::queue_chunk>&& chunk)
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    m_queue.emplace_back(std::move(chunk));
}

std::unique_ptr<marker::queue_chunk> marker::pop_queue_chunk()
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    if (m_queue.empty()) {
        non_blocking_trace_barrier_buffers();
        if (m_queue.empty()) {
            return std::unique_ptr<marker::queue_chunk>();
        }
    }
    auto chunk = std::move(m_queue.back());
    m_queue.pop_back();
    return chunk;
}

void marker::non_blocking_push(gc_untyped_ptr* p)
{
    if (m_queue.back()->is_full()) {
        m_queue.emplace_back(new queue_chunk());
    }
    m_queue.back()->push(p);
}

}}
