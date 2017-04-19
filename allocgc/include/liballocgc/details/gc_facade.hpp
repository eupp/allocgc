#ifndef ALLOCGC_GARBAGE_COLLECTOR_HPP
#define ALLOCGC_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details {

template <typename GCStrategy>
class gc_facade : private utils::noncopyable, private utils::nonmovable
{
public:
//    static inline void init()
//    {
//        logging::touch();
//        // same hack as with logger
//        allocators::memory_index::init();
//    }

    static inline gc_alloc::response allocate(const gc_alloc::request& rqst)
    {
        return strategy.allocate(rqst);
    }

    static inline void abort(const gc_alloc::response& rsp)
    {
        strategy.abort(rsp);
    }

    static inline void commit(const gc_alloc::response& rsp)
    {
        strategy.commit(rsp);
    }

    static inline void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
    {
        strategy.commit(rsp, type_meta);
    }

    static inline gc_offsets make_offsets(const gc_alloc::response& rsp)
    {
        return strategy.make_offsets(rsp);
    }

    static inline void register_handle(gc_handle& handle, byte* ptr)
    {
        strategy.register_handle(handle, ptr);
    }

    static inline void deregister_handle(gc_handle& handle)
    {
        strategy.deregister_handle(handle);
    }

    static inline byte* register_pin(const gc_handle& handle)
    {
        return strategy.register_pin(handle);
    }

    static inline void deregister_pin(byte* ptr)
    {
        strategy.deregister_pin(ptr);
    }

    static inline byte* push_pin(const gc_handle& handle)
    {
        return strategy.push_pin(handle);
    }

    static inline void pop_pin(byte* ptr)
    {
        strategy.pop_pin(ptr);
    }

    static inline byte* rbarrier(const gc_handle& handle)
    {
        return strategy.rbarrier(handle);
    }

    static inline void wbarrier(gc_handle& dst, const gc_handle& src)
    {
        strategy.wbarrier(dst, src);
    }

    static inline void interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
    {
        strategy.interior_wbarrier(handle, offset);
    }

    static inline bool compare(const gc_handle& a, const gc_handle& b)
    {
        gc_unsafe_scope unsafe_scope;
        return rbarrier(a) == rbarrier(b);
    }

    static inline void register_thread(const thread_descriptor& descr)
    {
        strategy.register_thread(descr);
    }

    static inline void deregister_thread(std::thread::id id)
    {
        strategy.deregister_thread(id);
    }

    static bool increment_heap_size(size_t alloc_size)
    {
        std::unique_lock<std::mutex> lock(heap_mutex);
        size_t size = heap_size + alloc_size;
        if (size > COLLECT_THRESHOLD * heap_limit) {
            return false;
        }
        else if (size > MARK_THRESHOLD * heap_limit) {
            gc_options opt;
            opt.kind = gc_kind::CONCURRENT_MARK;
            opt.gen  = 0;

            lock.unlock();
            initiation_point(initiation_point_type::HEAP_LIMIT_EXCEEDED, opt);
            lock.lock();
        }
        heap_size += alloc_size;
        return true;
    }

    static void decrement_heap_size(size_t size)
    {
        std::lock_guard<std::mutex> lock(heap_mutex);
        heap_size -= size;
    }

    static void set_heap_limit(size_t size)
    {
        std::lock_guard<std::mutex> lock(heap_mutex);
        heap_limit = (size == std::numeric_limits<size_t>::max()) ? HEAP_START_LIMIT : size;
        heap_maxlimit = size;
    }

    static void expand_heap()
    {
        std::lock_guard<std::mutex> lock(heap_mutex);
        size_t increased_size = INCREASE_FACTOR * heap_limit;
        heap_limit = std::min(increased_size, heap_maxlimit);
    }

    static void initiation_point(initiation_point_type ipt, const gc_options& opt)
    {
        gc_safe_scope safe_scope;
        std::lock_guard<std::mutex> lock(gc_mutex);

        if (ipt == initiation_point_type::USER_REQUEST) {
            logging::info() << "Thread initiates gc by user's request";
            gc(opt);
        } else if (ipt == initiation_point_type::HEAP_LIMIT_EXCEEDED) {
//        logging::info() << "Heap limit exceeded - thread initiates gc";
            gc(opt);
        } else if (ipt == initiation_point_type::CONCURRENT_MARKING_FINISHED) {
            logging::info() << "Concurrent marking finished - Thread initiates gc";
            gc(opt);
        } else if (ipt == initiation_point_type::START_MARKING) {
            gc(opt);
        } else if (ipt == initiation_point_type::START_COLLECTING) {
            gc(opt);
        }
    }

    static inline gc_stat stats()
    {
        gc_stat stats = {
            .gc_count   = gc_cnt,
            .gc_time    = gc_time
        };
        return stats;
    }
private:
    gc_facade() = delete;

    static void gc(const gc_options& opt)
    {
        if (!check_gc_kind(opt.kind)) {
            return;
        }

        gc_run_stat run_stats = strategy.gc(opt);
        if (run_stats.pause_stat.type == gc_pause_type::SKIP) {
            return;
        }

        logging::info() << "GC pause (" << gc_pause_type_to_str(run_stats.pause_stat.type)
                        << ") duration "<< duration_to_str(run_stats.pause_stat.duration);
        register_gc_run(run_stats);
    }

    static bool check_gc_kind(gc_kind kind)
    {
        if (kind == gc_kind::CONCURRENT_MARK && !strategy.info().incremental_flag) {
            return false;
        }
        return true;
    }

    static void register_gc_run(const gc_run_stat& stats)
    {
        ++gc_cnt;
        gc_time += stats.pause_stat.duration;
    }

    static bool is_interior_pointer(const gc_handle& handle, byte* iptr)
    {
        using namespace allocators;

        byte* ptr = rbarrier(handle);
        gc_memory_descriptor* descr = memory_index::get_descriptor(ptr).to_gc_descriptor();
        byte* cell_begin = descr->cell_start(ptr);
        byte* cell_end   = cell_begin + descr->cell_size(ptr);
        return (cell_begin <= iptr) && (iptr < cell_end);
    }

    static bool is_interior_offset(const gc_handle& handle, ptrdiff_t shift)
    {
        return is_interior_pointer(handle, rbarrier(handle) + shift);
    }

    static const size_t HEAP_START_LIMIT;

    static const double INCREASE_FACTOR;
    static const double MARK_THRESHOLD;
    static const double COLLECT_THRESHOLD;

    static GCStrategy strategy;
    static std::mutex gc_mutex;
    static std::mutex heap_mutex;

    static size_t gc_cnt;
    static gc_clock::duration gc_time;

    static size_t heap_limit;
    static size_t heap_maxlimit;
    static size_t heap_size;
};

template <typename GCStrategy>
GCStrategy gc_facade<GCStrategy>::strategy{};

template <typename GCStrategy>
std::mutex gc_facade<GCStrategy>::gc_mutex{};

template <typename GCStrategy>
std::mutex gc_facade<GCStrategy>::heap_mutex{};

template <typename GCStrategy>
size_t gc_facade<GCStrategy>::gc_cnt = 0;

template <typename GCStrategy>
gc_clock::duration gc_facade<GCStrategy>::gc_time{0};

template <typename GCStrategy>
size_t gc_facade<GCStrategy>::heap_limit = 0;

template <typename GCStrategy>
size_t gc_facade<GCStrategy>::heap_maxlimit = 0;

template <typename GCStrategy>
size_t gc_facade<GCStrategy>::heap_size = 0;

template <typename GCStrategy>
const size_t gc_facade<GCStrategy>::HEAP_START_LIMIT = 4 * 1024 * 1024;

template <typename GCStrategy>
const double gc_facade<GCStrategy>::INCREASE_FACTOR = 2.0;

template <typename GCStrategy>
const double gc_facade<GCStrategy>::MARK_THRESHOLD = 0.6;

template <typename GCStrategy>
const double gc_facade<GCStrategy>::COLLECT_THRESHOLD = 1.0;

}}

#endif //ALLOCGC_GARBAGE_COLLECTOR_HPP
