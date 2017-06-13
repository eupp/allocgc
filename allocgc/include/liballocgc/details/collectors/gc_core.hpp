#ifndef ALLOCGC_GC_CORE_HPP
#define ALLOCGC_GC_CORE_HPP

#include <liballocgc/details/collectors/gc_heap.hpp>

#include <liballocgc/details/utils/make_unique.hpp>
#include <liballocgc/details/utils/utility.hpp>

#include <liballocgc/details/collectors/static_root_set.hpp>
#include <liballocgc/details/collectors/marker.hpp>

#include <liballocgc/details/threads/gc_thread_manager.hpp>
#include <liballocgc/details/threads/gc_thread_descriptor.hpp>

namespace allocgc { namespace details { namespace collectors {

template <typename Derived>
class gc_core : public gc_launcher, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_core(remset* rset)
        : m_marker(&m_packet_manager, rset)
        , m_heap(this)
        , m_threads_available(std::thread::hardware_concurrency())
        , m_gc_cnt(0)
        , m_gc_time(0)
        , m_tm(clock_t::now())
    {
        allocators::memory_index::init();
    }

    ~gc_core()
    {}

    gc_alloc::response allocate(const gc_alloc::request& rqst)
    {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rqst.buffer());

        // TODO: correct size check including gc_box meta-information
        gc_alloc::response rsp = rqst.alloc_size() <= LARGE_CELL_SIZE
            ? this_thread->allocate(rqst)
            : m_heap.allocate(rqst);

        stack_entry->obj_start = rsp.obj_start();
        stack_entry->obj_size  = rqst.alloc_size();
        stack_entry->meta_requested = rqst.type_meta() == nullptr;

        this_thread->register_stack_entry(stack_entry);

        return rsp;
    }

    void abort(const gc_alloc::response& rsp)
    {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
        this_thread->deregister_stack_entry(stack_entry);
    }

    void commit(const gc_alloc::response& rsp)
    {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());

        this_thread->deregister_stack_entry(stack_entry);

        assert(stack_entry->descriptor);
        stack_entry->descriptor->commit(rsp.cell_start());
    }

    void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
    {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());

        this_thread->deregister_stack_entry(stack_entry);

        assert(stack_entry->descriptor);
        stack_entry->descriptor->commit(rsp.cell_start(), type_meta);
    }

    gc_offsets make_offsets(const gc_alloc::response& rsp)
    {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
        return boost::make_iterator_range(stack_entry->offsets.begin(), stack_entry->offsets.end());
    }

    byte* rbarrier(const gc_handle& handle)
    {
        return gc_handle_access::get<std::memory_order_relaxed>(handle);
    }

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
    {
        gc_handle_access::advance<std::memory_order_relaxed>(handle, offset);
    }

    void register_handle(gc_handle& handle, byte* ptr)
    {
        using namespace allocators;

        memory_descriptor descr = memory_index::get_descriptor(reinterpret_cast<byte*>(&handle));
        gc_handle_access::set<std::memory_order_relaxed>(handle, ptr);
        if (descr.is_stack_descriptor()) {
            this_thread->register_root(&handle);
        } else if (descr.is_null()) {
            m_static_roots.register_root(&handle);
        } else {
            if (this_thread) {
                this_thread->register_heap_ptr(&handle);
            }
        }
    }

    void deregister_handle(gc_handle& handle)
    {
        using namespace allocators;

        memory_descriptor descr = memory_index::get_descriptor(reinterpret_cast<byte*>(&handle));
        if (descr.is_stack_descriptor()) {
            this_thread->deregister_root(&handle);
        } else if (descr.is_null()) {
            m_static_roots.deregister_root(&handle);
        } else {
            if (this_thread) {
                this_thread->deregister_heap_ptr(&handle);
            }
        }
    }

    byte* register_pin(const gc_handle& handle)
    {
        gc_unsafe_scope unsafe_scope;
        byte* ptr = rbarrier(handle);
        if (ptr) {
            this_thread->register_pin(ptr);
        }
        return ptr;
    }

    void  deregister_pin(byte* pin)
    {
        if (pin) {
            this_thread->deregister_pin(pin);
        }
    }

    byte* push_pin(const gc_handle& handle)
    {
        gc_unsafe_scope unsafe_scope;
        byte* ptr = rbarrier(handle);
        if (ptr) {
            this_thread->push_pin(ptr);
        }
        return ptr;
    }

    void  pop_pin(byte* pin)
    {
        if (pin) {
            this_thread->pop_pin(pin);
        }
    }

    void register_thread(const thread_descriptor& descr)
    {
        using namespace threads;

//        assert(descr.id == std::this_thread::get_id());

        std::unique_ptr<gc_thread_descriptor> gc_thrd_descr
                = utils::make_unique<gc_thread_descriptor>(descr, m_heap.allocate_tlab(descr.id));

        this_thread = gc_thrd_descr.get();
        m_thread_manager.register_thread(descr.id, std::move(gc_thrd_descr));
    }

    void deregister_thread(std::thread::id id)
    {
        assert(id == std::this_thread::get_id());

        m_thread_manager.deregister_thread(id);
    }

    gc_runstat gc(const gc_options& options) override
    {
        gc_safe_scope safe_scope;
        std::lock_guard<std::mutex> lock(m_mutex);

        before_gc(options);

        gc_runstat stat = static_cast<Derived*>(this)->gc_impl(options);

        after_gc(options, stat);

        return stat;
    }

    gc_stat stats()
    {
        gc_stat stats = {
                .gc_count   = m_gc_cnt,
                .gc_time    = m_gc_time,
                .gc_mem     = m_heap.stats()
        };
        return stats;
    }

    size_t threads_available() const
    {
        return m_threads_available;
    }

    void set_threads_available(size_t threads_available)
    {
        m_threads_available = threads_available;
    }

    void set_heap_limit(size_t limit)
    {
        m_heap.set_limit(limit);
    }
protected:
    threads::world_snapshot stop_the_world()
    {
        return m_thread_manager.stop_the_world();
    }

    void trace_roots(const threads::world_snapshot& snapshot)
    {
        snapshot.trace_roots([this] (gc_handle* root) { root_trace_cb(root); });
        m_static_roots.trace([this] (gc_handle* root) { root_trace_cb(root); });
    }

    void trace_pins(const threads::world_snapshot& snapshot)
    {
        snapshot.trace_pins([this] (byte* ptr) { pin_trace_cb(ptr); });
    }

    void trace_uninit(const threads::world_snapshot& snapshot)
    {
        snapshot.trace_uninit([this] (byte* obj_start, size_t obj_size) { conservative_obj_trace_cb(obj_start, obj_size); });
    }

    void trace_remset()
    {
        m_marker.trace_remset();
    }

    void start_concurrent_marking(size_t threads_available)
    {
        m_marker.concurrent_mark(threads_available);
    }
    void start_marking()
    {
        m_marker.mark();
    }

    gc_collect_stat collect(const threads::world_snapshot& snapshot, size_t threads_available)
    {
        return m_heap.collect(snapshot, threads_available, &m_static_roots);
    }

    void shrink()
    {
        m_heap.shrink();
    }
private:
    void before_gc(const gc_options& options)
    {
        if (options.kind == gc_kind::LAUNCH_CONCURRENT_MARK) {
            return;
        }

//        std::cerr << "GC start time: "
//                  << std::chrono::duration_cast<std::chrono::milliseconds>(clock_t::now() - m_tm).count()
//                  << std::endl;
//
        print_gc_mem_stats(stats().gc_mem);
    }

    void after_gc(const gc_options& options, const gc_runstat& runstats)
    {
        if (options.kind == gc_kind::LAUNCH_CONCURRENT_MARK) {
            return;
        }

        logging::info() << "GC kind (" << gc_kind_to_str(options.kind)
                        << ") pause "  << duration_to_str(runstats.pause);

        print_gc_mem_stats(stats().gc_mem);
        print_gc_run_stats(options, runstats);

        ++m_gc_cnt;
        m_gc_time += runstats.pause;

//        std::cerr << "GC finish time: "
//                  << std::chrono::duration_cast<std::chrono::milliseconds>(clock_t::now() - m_tm).count()
//                  << std::endl;
    }

    void root_trace_cb(gc_handle* root)
    {
        byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
        if (ptr) {
            gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
            cell.set_mark(true);
            m_marker.add_root(cell);

            logging::debug() << "root: " << (void*) root /* << "; point to: " << (void*) obj_start */;
        }
    }

    void pin_trace_cb(byte* ptr)
    {
        if (ptr) {
            gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
            cell.set_mark(true);
            cell.set_pin(true);
            m_marker.add_root(cell);

            logging::debug() << "pin: " << (void*) ptr;
        }
    }

    void conservative_obj_trace_cb(byte* obj_start, size_t obj_size)
    {
        assert(obj_start && obj_size > 0);
        gc_cell cell = allocators::memory_index::get_gc_cell(obj_start);
        cell.set_mark(true);
        cell.set_pin(true);

        logging::info() << "uninitialized object: " << (void*) obj_start /* << "; point to: " << (void*) obj_start */;

        gc_handle* begin = reinterpret_cast<gc_handle*>(obj_start);
        gc_handle* end   = reinterpret_cast<gc_handle*>(obj_start + obj_size);
        for (gc_handle* it = begin; it < end; ++it) {
            byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*it);
            cell = allocators::memory_index::get_gc_cell(ptr);
            if (!cell.get_mark() && cell.is_init()) {
                cell.set_mark(true);
                cell.set_pin(true);
                m_marker.add_root(cell);

                logging::debug() << "pointer from uninitialized object: " << (void*) ptr /* << "; point to: " << (void*) obj_start */;
            }
        }
    }

    void print_gc_mem_stats(const gc_memstat& stats)
    {
        std::cerr << "****************************************************************************" << std::endl;
        std::cerr << "   GC HEAP SUMMARY                                                          " << std::endl;
        std::cerr << "mem live:  " << heapsize_to_str(stats.mem_live, 0) << std::endl;
        std::cerr << "mem used:  " << heapsize_to_str(stats.mem_used, 0) << std::endl;
        std::cerr << "mem extra: " << heapsize_to_str(stats.mem_extra, 0) << std::endl;
        std::cerr << "****************************************************************************" << std::endl;
    }

    void print_gc_run_stats(const gc_options& options, const gc_runstat& stats)
    {
        if (options.kind == gc_kind::LAUNCH_CONCURRENT_MARK) {
            return;
        }

        std::cerr << "****************************************************************************" << std::endl;
        std::cerr << "   GC RUN SUMMARY                                                           " << std::endl;
        std::cerr << "gc kind:     " << gc_kind_to_str(options.kind) << std::endl;
        std::cerr << "pause time:  " << duration_to_str(stats.pause, 0) << std::endl;
        std::cerr << "freed:       " << heapsize_to_str(stats.collection.mem_freed, 0) << std::endl;
        std::cerr << "moved:       " << heapsize_to_str(stats.collection.mem_moved, 0) << std::endl;
        std::cerr << "****************************************************************************" << std::endl;
    }

    static thread_local threads::gc_thread_descriptor* this_thread;

    typedef std::chrono::steady_clock clock_t;

    threads::gc_thread_manager m_thread_manager;
    static_root_set m_static_roots;
    packet_manager m_packet_manager;
    marker m_marker;
    gc_heap m_heap;
    size_t m_threads_available;
    size_t m_gc_cnt;
    gc_clock::duration m_gc_time;
    std::mutex m_mutex;
    clock_t::time_point m_tm;
};

template <typename Derived>
thread_local threads::gc_thread_descriptor* gc_core<Derived>::this_thread = nullptr;

}}}

#endif //ALLOCGC_GC_CORE_HPP
