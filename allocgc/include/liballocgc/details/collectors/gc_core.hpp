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

template <typename Tag>
class gc_core : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_core(gc_launcher* launcher, remset* rset)
        : m_marker(&m_packet_manager, rset)
        , m_heap(launcher)
        , m_threads_available(std::thread::hardware_concurrency())
        , m_gc_cnt(0)
        , m_gc_time(0)
    {
        allocators::memory_index::init();

        m_conservative_mode = false;
    }

    ~gc_core()
    {}

    gc_alloc::response allocate(const gc_alloc::request& rqst)
    {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rqst.buffer());
        stack_entry->zeroing_flag = !m_conservative_mode;

        gc_alloc::response rsp = m_heap.allocate(rqst);

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

        assert(descr.id == std::this_thread::get_id());

        std::unique_ptr<gc_thread_descriptor> gc_thrd_descr
                = utils::make_unique<gc_thread_descriptor>(descr);

        this_thread = gc_thrd_descr.get();
        m_thread_manager.register_thread(descr.id, std::move(gc_thrd_descr));
    }

    void deregister_thread(std::thread::id id)
    {
        assert(id == std::this_thread::get_id());

        m_thread_manager.deregister_thread(id);
    }

    gc_stat stats()
    {
        gc_stat stats = {
                .gc_count   = m_gc_cnt,
                .gc_time    = m_gc_time
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
        if (m_conservative_mode) {
            snapshot.trace_roots([this] (gc_handle* root) { conservative_root_trace_cb(root); });
        } else {
            snapshot.trace_roots([this] (gc_handle* root) { root_trace_cb(root); });
        }
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

    gc_heap_stat collect(const threads::world_snapshot& snapshot, size_t threads_available)
    {
        return m_heap.collect(snapshot, threads_available, &m_static_roots);
    }

    void shrink()
    {
        m_heap.shrink();
    }

    void register_gc_run(const gc_run_stat& stats)
    {
        if (stats.pause_stat.type == gc_pause_type::SKIP) {
            return;
        }

        logging::info() << "GC pause (" << gc_pause_type_to_str(stats.pause_stat.type)
                        << ") duration "<< duration_to_str(stats.pause_stat.duration);

        ++m_gc_cnt;
        m_gc_time += stats.pause_stat.duration;
    }
private:
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

    void conservative_root_trace_cb(gc_handle* root)
    {
        byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
        if (ptr) {
            gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
            if (!cell.get_mark() && cell.is_init()) {
                cell.set_mark(true);
                cell.set_pin(true);
                m_marker.add_root(cell);

                logging::debug() << "root: " << (void*) root << "; point to: " << (void*) ptr;
            }
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

    static thread_local threads::gc_thread_descriptor* this_thread;

    threads::gc_thread_manager m_thread_manager;
    static_root_set m_static_roots;
    packet_manager m_packet_manager;
    marker m_marker;
    gc_heap m_heap;
    size_t m_threads_available;
    size_t m_gc_cnt;
    gc_clock::duration m_gc_time;
    bool m_conservative_mode;
};

template <typename Tag>
thread_local threads::gc_thread_descriptor* gc_core<Tag>::this_thread = nullptr;

}}}

#endif //ALLOCGC_GC_CORE_HPP
