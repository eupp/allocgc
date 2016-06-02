#ifndef DIPLOMA_MARKER_HPP
#define DIPLOMA_MARKER_HPP

#include <vector>
#include <stack>
#include <mutex>
#include <memory>
#include <condition_variable>

#include <libprecisegc/details/threads/world_state.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/trace_ptr.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

namespace precisegc { namespace details {

class marker
{
public:
    marker();

    void trace_roots(const threads::world_state& wstate);
    void trace_pins(const threads::world_state& wstate);
    void trace_barrier_buffers();

    void start_marking();
    void pause_marking();

    void join_markers();
    void mark();

    void wait_for_marking();
private:
    class queue_chunk : private utils::noncopyable, private utils::nonmovable
    {
    public:
        queue_chunk();

        void push(ptrs::gc_untyped_ptr* p);
        ptrs::gc_untyped_ptr* pop();

        bool is_full() const;
        bool empty() const;
    private:
        static const size_t SIZE = 4096;
        ptrs::gc_untyped_ptr* m_data[SIZE];
        size_t m_size;
    };

    class worker : private utils::noncopyable
    {
    public:
        static void routine(marker* m);

        worker(marker* m);

        worker(worker&&) = default;
        worker& operator=(worker&&) = default;

        void mark();

        void push(void* p);
        bool pop(void*& p);
    private:
        static const size_t LOCAL_QUEUE_SIZE = 2;

//        std::unique_ptr<queue_chunk> m_local_queue[LOCAL_QUEUE_SIZE];
//        size_t m_curr_queue;
        std::vector<void*> m_local_stack;
        marker* m_marker;
    };

    void non_blocking_trace_barrier_buffers();

//    void push_queue_chunk(std::unique_ptr<queue_chunk>&& chunk);
//    std::unique_ptr<queue_chunk> pop_queue_chunk();

    void non_blocking_push(void* p);

    std::vector<void*> m_stack;
    std::mutex m_stack_mutex;

    size_t m_markers_cnt;
    std::mutex m_markers_mutex;
    std::condition_variable m_markers_cond;
    std::vector<utils::scoped_thread> m_workers;
    std::atomic<bool> m_mark_flag;
};

}}

#endif //DIPLOMA_MARKER_HPP
