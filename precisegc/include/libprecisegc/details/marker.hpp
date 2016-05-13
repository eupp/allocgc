#ifndef DIPLOMA_MARKER_HPP
#define DIPLOMA_MARKER_HPP

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/gc_untyped_ptr.h>
#include <libprecisegc/details/utils/scoped_thread.hpp>

namespace precisegc { namespace details {

class marker
{
public:
    marker() = default;

    void trace_roots();

    void start_marking();
    void pause_marking();

    void wait_for_marking();
private:
    class queue_chunk : private noncopyable, private nonmovable
    {
    public:
        queue_chunk();

        void push(gc_untyped_ptr* p);
        gc_untyped_ptr* pop();

        bool is_full() const;
    private:
        static const size_t SIZE = 4096;
        gc_untyped_ptr m_data[SIZE];
        size_t m_size;
    };

    class worker : private noncopyable
    {
    public:
        static void routine();

        worker();

        worker(worker&&) = default;
        worker& operator=(worker&&) = default;

        void mark();
    private:
        void trace(gc_untyped_ptr* p);

        void push(gc_untyped_ptr* p);
        gc_untyped_ptr* pop();

        static const size_t LOCAL_QUEUE_SIZE = 2;

        std::unique_ptr<queue_chunk> m_local_queue[LOCAL_QUEUE_SIZE];
        size_t m_curr_queue;
    };

    std::vector<std::unique_ptr<queue_chunk>> m_queue;
    std::mutex m_queue_mutex;

    size_t m_markers_cnt;
    std::mutex m_markers_mutex;
    std::condition_variable m_markers_cond;
    std::vector<utils::scoped_thread> m_workers;
};

}}

#endif //DIPLOMA_MARKER_HPP
