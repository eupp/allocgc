#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <condition_variable>

#ifdef BDW_GC
#define GC_THREADS
    #include <gc/gc.h>
#endif

#ifdef PRECISE_GC
    #include <liballocgc/liballocgc.hpp>
    #include <liballocgc/details/threads/managed_thread.hpp>
using namespace precisegc;
#endif

#include <liballocgc/details/utils/scoped_thread.hpp>
#include <liballocgc/details/utils/scope_guard.hpp>

#include "../../common/macro.hpp"
#include "../../common/timer.hpp"

template <typename Function, typename... Args>
std::thread create_thread(Function&& f, Args&&... args)
{
#ifdef PRECISE_GC
    return gc_thread::create(std::forward<Function>(f), std::forward<Args>(args)...);
#else
    return std::thread(std::forward<Function>(f), std::forward<Args>(args)...);
#endif
};

static const size_t QUEUE_SIZE  = 512;
static const size_t PACKET_SIZE = 32;
static const size_t TOTAL_WORK  = 4096 * QUEUE_SIZE;

struct work_packet
{
    int m_data[PACKET_SIZE];
};

struct pc_queue
{
    pc_queue()
        : m_size(0)
    {}

    void push(ptr_in(work_packet) p)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_full_cond.wait(lock, [this] { return m_size < QUEUE_SIZE; });
        m_queue[m_size++] = p;
        if (m_size == 1) {
//            std::cout << "oopppss" << std::endl;
            m_empty_cond.notify_one();
        }
    }

    ptr_t(work_packet) pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_empty_cond.wait(lock, [this] { return m_size > 0; });
        ptr_t(work_packet) p = m_queue[--m_size];
        set_null(m_queue[m_size]);
        if (m_size == QUEUE_SIZE - 1) {
//            std::cout << "oopppss" << std::endl;
            m_full_cond.notify_one();
        }
        return p;
    }

    size_t m_size;
    std::mutex m_mutex;
    std::condition_variable m_empty_cond;
    std::condition_variable m_full_cond;
    ptr_t(work_packet) m_queue[QUEUE_SIZE];
};

void producer_routine(pc_queue* queue)
{
    #ifdef BDW_GC
        GC_stack_base sb;
        GC_get_stack_base(&sb);
        GC_register_my_thread(&sb);
        auto guard = allocgc::details::utils::make_scope_guard([] { GC_unregister_my_thread(); });
    #endif

    for (size_t n = 0; n < TOTAL_WORK; ++n) {
        ptr_t(work_packet) packet = new_(work_packet);
        pin_t(work_packet) pin_packet = pin(packet);
        memset(pin_packet->m_data, 0, PACKET_SIZE);
//        for (size_t i = 0; i < PACKET_SIZE; ++i) {
//            pin_packet->m_data[i] = rand();
//        }
        queue->push(packet);
    }
}

void consumer_routine(pc_queue* queue)
{
    #ifdef BDW_GC
        GC_stack_base sb;
        GC_get_stack_base(&sb);
        GC_register_my_thread(&sb);
        auto guard = allocgc::details::utils::make_scope_guard([] { GC_unregister_my_thread(); });
    #endif

    for (size_t n = 0; n < TOTAL_WORK; ++n) {
        if (n % (TOTAL_WORK / 10) == 0) {
            std::cout << n * 100 / TOTAL_WORK << "%" << std::endl;
        }

        ptr_t(work_packet) packet = queue->pop();
        pin_t(work_packet) pin_packet = pin(packet);
//        std::sort(pin_packet->m_data, pin_packet->m_data + PACKET_SIZE);

        delete_(packet);
        set_null(packet);
    }
}

int main(int argc, const char* argv[])
{
    bool incremental_flag = false;
    bool compacting_flag = false;
    for (int i = 1; i < argc; ++i) {
        auto arg = std::string(argv[i]);
        if (arg == "--incremental") {
            incremental_flag = true;
        } else if (arg == "--compacting") {
            compacting_flag = true;
        }
    }

#if defined(PRECISE_GC)
    gc_init_options ops;
//    ops.heapsize = 4 * 1024 * 1024;      // 4 Mb
    ops.threads_available = 1;
    ops.algo = incremental_flag ? gc_algo::INCREMENTAL : gc_algo::SERIAL;
    ops.initiation = gc_initiation::SPACE_BASED;
    ops.compacting = compacting_flag ? gc_compacting::ENABLED : gc_compacting::DISABLED;
    ops.loglevel = gc_loglevel::SILENT;
    ops.print_stat = false;
    gc_init(ops);
#elif defined(BDW_GC)
    GC_INIT();
        GC_allow_register_threads();
        if (incremental_flag) {
            GC_enable_incremental();
        }
#endif

    std::cout << "Producer-Consumer queue test " << std::endl;
    std::cout << "Size of queue " << QUEUE_SIZE * sizeof(work_packet) << " b" << std::endl;
    std::cout << "Total memory usage " << TOTAL_WORK * sizeof(work_packet) << " b" << std::endl;

    typedef precisegc::details::utils::scoped_thread thread_t;
    pc_queue queue;
    thread_t consumer_thread = create_thread(consumer_routine, &queue);
    thread_t producer_thread = create_thread(producer_routine, &queue);

    timer tm;
    consumer_thread.join();

    std::cout << "Completed in " << tm.elapsed<std::chrono::milliseconds>() << " ms" << std::endl;
#if defined(BDW_GC)
    std::cout << "Completed " << GC_get_gc_no() << " collections" << std::endl;
        std::cout << "Heap size is " << GC_get_heap_size() << std::endl;
#elif defined(PRECISE_GC)
    gc_stat stat = gc_stats();
    std::cout << "Completed " << stat.gc_count << " collections" << std::endl;
    std::cout << "Time spent in gc " << std::chrono::duration_cast<std::chrono::milliseconds>(stat.gc_time).count() << " ms" << std::endl;
    std::cout << "Average pause time " << std::chrono::duration_cast<std::chrono::microseconds>(stat.gc_time / stat.gc_count).count() << " us" << std::endl;
#endif
}